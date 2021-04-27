

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/IR/Verifier.h>
#include <remill/Arch/Arch.h>
#include <remill/Arch/Name.h>
#include <remill/BC/IntrinsicTable.h>
#include <remill/BC/Lifter.h>
#include <remill/BC/Optimizer.h>
#include <remill/BC/Util.h>
#include <remill/OS/OS.h>

#include <fstream>
#include <iostream>
#include <map>

// For gflags definitions
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"

DEFINE_uint64(code_address, 0,
              "Start address of the code section");
DEFINE_string(code_filename, "",
              "Filename of raw code section data");

DEFINE_string(ir_out, "", "Path to file where the LLVM IR should be saved.");
DEFINE_string(bc_out, "",
              "Path to file where the LLVM bitcode should be "
              "saved.");

DEFINE_string(intrinsics_filename, INTRINSICS_BC, "Llvm bitcode containing "
              "the intrinsics. "
              "They will be copied to generated module and force-inlined.");
DEFINE_string(basic_blocks_filename, "",
              "Filename of a file containing basic block addresses.");
DEFINE_string(name_map_filename, "",
              "Filename of a file containing name map.");

#pragma clang diagnostic pop

using Memory = std::unordered_map<uint64_t, uint8_t>;

static Memory LoadCode() {
  // mock code
  Memory mem;
  // C7053713000028020000C3
  // 31C064A300000000C3
  std::ifstream f(FLAGS_code_filename, std::ios_base::in | std::ios_base::binary);
  if (!f.is_open()) {
    std::cerr << "Cannot open code file" << std::endl;
    exit(1);
  }
  f.seekg(0, std::ios_base::end);
  auto size = f.tellg();
  f.seekg(0);
  mem.reserve(size);
  for (std::uint64_t i = FLAGS_code_address; i < FLAGS_code_address + size; i++) {
    char b;
    f.read(&b, 1);
    mem.insert(std::make_pair(i, b));
  }

  //mem.insert(std::make_pair(9, 0x00));
  //mem.insert(std::make_pair(10, 0xc3));
  return mem;
}

static std::vector<uint64_t> LoadTraceHeadAddresses() {
  // mock code
  std::vector<uint64_t> res;
  std::ifstream f(FLAGS_basic_blocks_filename, std::ios_base::in);
  if (!f.is_open()) {
    throw std::runtime_error("Can't open basic blocks file");
  }

  while (true) {
    std::uint64_t r;
    f >> r;
    if (f.eof()) break;
    res.push_back(r);
  }
  return res;
}

static std::unordered_map<uint64_t, std::string> LoadNameMap() {
  if (FLAGS_name_map_filename.empty())
    return {};

  std::unordered_map<uint64_t, std::string> res;
  std::ifstream f(FLAGS_name_map_filename, std::ios_base::in);
  if (!f.is_open()) {
    throw std::runtime_error("Can't open name map file");
  }

  while (true) {
    std::uint64_t addr;
    std::string name;
    f >> addr >> name;
    if (f.eof()) break;
    res.emplace(addr, name);
  }
  return res;
}

class SimpleTraceManager : public remill::TraceManager {
 public:
  ~SimpleTraceManager() override = default;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "VirtualCallInCtorOrDtor"
  template<typename Generator>
  explicit SimpleTraceManager(
      llvm::Module *module,
      Memory &memory_,
      Generator const& trace_heads_generator,
      std::unordered_map<std::uint64_t, std::string> const& name_map_)
      : memory(memory_), name_map(name_map_) {
    for (auto const& addr : trace_heads_generator)
    {
      traces[addr].function = remill::DeclareLiftedFunction(module, TraceName(addr));
    }
  }
#pragma clang diagnostic pop

  std::string TraceName(uint64_t addr) override {
    auto it = name_map.find(addr);
    std::stringstream ss;
    ss << "lifted_";

    if (it != name_map.end())
      ss << it->second << "_";
    ss << std::hex << addr;

    return ss.str();
  }

 protected:
  // Called when we have lifted, i.e. defined the contents, of a new trace.
  // The derived class is expected to do something useful with this.
  void SetLiftedTraceDefinition(uint64_t addr,
                                llvm::Function *lifted_func) override {
    auto& trace = traces[addr];
    assert(trace.function == lifted_func || trace.function == nullptr);
    trace.function = lifted_func;
    trace.lifted = true;
  }

  // Get a declaration for a lifted trace. The idea here is that a derived
  // class might have additional global info available to them that lets
  // them declare traces ahead of time. In order to distinguish between
  // stuff we've lifted, and stuff we haven't lifted, we allow the lifter
  // to access "defined" vs. "declared" traces.
  //
  // NOTE: This is permitted to return a function from an arbitrary module.
  llvm::Function *GetLiftedTraceDeclaration(uint64_t addr) override {
    auto trace_it = traces.find(addr);
    if (trace_it != traces.end()) {
      return trace_it->second.function;
    } else {
      return nullptr;
    }
  }

  // Get a definition for a lifted trace.
  //
  // Used by TraceLifter to check whether the trace was already lifted (only)
  //
  // NOTE: This is permitted to return a function from an arbitrary module.
  llvm::Function *GetLiftedTraceDefinition(uint64_t addr) override {
    auto trace_it = traces.find(addr);
    if (trace_it != traces.end() && trace_it->second.lifted) {
      return trace_it->second.function;
    } else {
      return nullptr;
    }
  }

  // Try to read an executable byte of memory. Returns `true` of the byte
  // at address `addr` is executable and readable, and updates the byte
  // pointed to by `byte` with the read value.
  bool TryReadExecutableByte(uint64_t addr, uint8_t *byte) override {
    auto byte_it = memory.find(addr);
    if (byte_it != memory.end()) {
      *byte = byte_it->second;
      return true;
    } else {
      return false;
    }
  }

 public:
  struct Trace {
    llvm::Function* function{nullptr};
    bool lifted{false};
  };

  std::unordered_map<std::uint64_t, llvm::Function*> GetDeclaredTraces() {
      decltype(GetDeclaredTraces()) res;
      for (auto& trace : traces) {
        if (trace.second.lifted) {
          res.emplace(trace.first, trace.second.function);
        }
      }
      return res;
  }

  Memory &memory;
  std::unordered_map<uint64_t, Trace> traces;
  std::unordered_map<std::uint64_t, std::string> const& name_map;
};

int main(int argc, char** argv) {
  google::SetVersionString(":shrug:");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  FLAGS_stderrthreshold = 0;

  google::SetCommandLineOption("arch", "x86");

  //addOccurrence

  auto& opts = llvm::cl::getRegisteredOptions();
  opts["opt-bisect-limit"]->addOccurrence(0, "opt-bisect-limit", "-1");
  llvm::cl::PrintOptionValues();

  //llvm::cl::

  if (FLAGS_intrinsics_filename.empty()) {
    std::cerr << "Please, specify --intrinsics_filename" << std::endl;
    return EXIT_FAILURE;
  }

  llvm::LLVMContext context;
  auto arch = remill::Arch::Build(&context, remill::OSName::kOSWindows,
                                  remill::ArchName::kArchX86);

  std::unique_ptr<llvm::Module> module(remill::LoadArchSemantics(arch));

  //const auto state_ptr_type = remill::StatePointerType(module.get());
  //const auto mem_ptr_type = remill::MemoryPointerType(module.get());

  Memory memory = LoadCode();

  auto trace_heads = LoadTraceHeadAddresses();
  auto name_map = LoadNameMap();

  SimpleTraceManager manager(module.get(), memory, trace_heads, name_map);
  remill::IntrinsicTable intrinsics(module);
  remill::InstructionLifter inst_lifter(arch, intrinsics);
  remill::TraceLifter trace_lifter(inst_lifter, manager);

  // Lift all discoverable traces with addresses taken from file
  for (auto addr : trace_heads) {
    trace_lifter.Lift(addr);
  }

  // Optimize the module, but with a particular focus on only the functions
  // that we actually lifted.
  remill::OptimizationGuide guide = {};
  guide.eliminate_dead_stores = true;
  remill::OptimizeModule(arch, module, manager.GetDeclaredTraces(), guide);


  // Create a new module in which we will move all the lifted functions. Prepare
  // the module for code of this architecture, i.e. set the data layout, triple,
  // etc.
  std::unique_ptr<llvm::Module> intermediate_module =
      std::make_unique<llvm::Module>("lifted_code", context);

  arch->PrepareModuleDataLayout(intermediate_module);

  // Move the lifted code into a new module. This module will be much smaller
  // because it won't be bogged down with all of the semantics definitions.
  // This is a good JITing strategy: optimize the lifted code in the semantics
  // module, move it to a new module, instrument it there, then JIT compile it.
  for (auto &lifted_entry : manager.traces) {
    remill::MoveFunctionIntoModule(lifted_entry.second.function, intermediate_module.get());
  }

  auto dispatcher_fun = llvm::Function::Create(arch->LiftedFunctionType(),
                                                 llvm::GlobalValue::LinkageTypes::ExternalLinkage,
                                               "uwin_xcute_remill_dispatch_recompiled",
                                               *intermediate_module);
 {

    auto args_ptr = dispatcher_fun->arg_begin();
    auto state = args_ptr++;
    auto pc = args_ptr++;
    auto mem = args_ptr++;

    std::vector<llvm::Value*> args;
    args.push_back(state);
    args.push_back(pc);
    args.push_back(mem);

    llvm::IRBuilder<> builder(context);

    auto block = llvm::BasicBlock::Create(context, "entry", dispatcher_fun);


    auto abort = llvm::BasicBlock::Create(context, "abort");
    {
      builder.SetInsertPoint(abort);
      builder.CreateRet(builder.CreateCall(intrinsics.error, args));
    }

    {
      builder.SetInsertPoint(block);

      auto sw = builder.CreateSwitch(pc, abort, manager.traces.size());
      for (auto bb : manager.traces) {
        std::stringstream hexbbss;
        hexbbss << std::hex << bb.first;
        std::string hexbb = hexbbss.str();

        auto call = llvm::BasicBlock
        ::Create(context,
                 "call_" + hexbb);

        builder.SetInsertPoint(call);

        auto fun = intermediate_module->getFunction(manager.TraceName(bb.first));
                          //arch->LiftedFunctionType());
        auto newmem = builder.CreateCall(fun, args);

        // do not expose the sub_* functions
        // TODO: do it only when compiling release?
        // TODO: is Private any better than InternalLinkage? Does it give any optimization chances?
        fun->setLinkage(llvm::GlobalValue::InternalLinkage);

        builder.CreateRet(newmem);

        call->insertInto(dispatcher_fun);

        sw->addCase(builder.getInt32(bb.first), call);
      }
    }
    abort->insertInto(dispatcher_fun);
  }

  auto intrinsics_module
      = remill::LoadModuleFromFile(&context, FLAGS_intrinsics_filename);

  // Here we do some voodoo magic. We link modules with different target
  // triples and data layouts. But this is okay, as there are no pointers
  // inside remill-generated code besides State& and Memory*. They are fine,
  // as they are using fixed-size types, ensuring no padding in-between
  // structure elements, and avoiding arch-specific types like long double.
  intermediate_module->setDataLayout(intrinsics_module->getDataLayout());
  intermediate_module->setTargetTriple(intrinsics_module->getTargetTriple());
  llvm::Linker::linkModules(*intrinsics_module, std::move(intermediate_module));

  guide.slp_vectorize = false;
  guide.loop_vectorize = false;
  guide.eliminate_dead_stores = false;
  guide.verify_input = false;

  remill::OptimizeBareModule(intrinsics_module, guide);

  int ret = EXIT_SUCCESS;

  // remove the (now inlined) intrinsics and trace functions, not to pollute the global namespace
  // also mark all functions with uwtable attribute to allow C++ exceptions to pass through
  {
    std::vector<std::string> rmnames;
    for (auto &fun : intrinsics_module->functions()) {
      auto nm = fun.getName().str();
      if (nm.rfind("__remill_", 0) == 0) {
        rmnames.emplace_back(std::move(nm));
      }

      fun.addFnAttr(llvm::Attribute::UWTable);
    }
    for (auto& nm : rmnames) {
      auto fun = intrinsics_module->getFunction(nm);
      if (fun->uses().empty())
      {}//fun->eraseFromParent();
      else {
        if (!fun->isDeclaration())
          LOG(WARNING) << "Can't remove " << nm << ", as it has some uses";
        else {
          LOG(ERROR) << "Intrinsic " << nm << " does not have implementation";
          ret = EXIT_FAILURE;
        }
      }
    }
  }


  if (!FLAGS_ir_out.empty()) {
    if (!remill::StoreModuleIRToFile(intrinsics_module.get(), FLAGS_ir_out, true)) {
      LOG(ERROR) << "Could not save LLVM IR to " << FLAGS_ir_out;
      ret = EXIT_FAILURE;
    }
  }
  if (!FLAGS_bc_out.empty()) {
    if (!remill::StoreModuleToFile(intrinsics_module.get(), FLAGS_bc_out, true)) {
      LOG(ERROR) << "Could not save LLVM bitcode to " << FLAGS_bc_out;
      ret = EXIT_FAILURE;
    }
  }

  return ret;

}