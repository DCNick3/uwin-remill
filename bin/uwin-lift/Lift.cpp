

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Linker/Linker.h>
#include <remill/BC/Lifter.h>
#include <remill/BC/Util.h>
#include <remill/BC/IntrinsicTable.h>
#include <remill/BC/Optimizer.h>
#include <remill/Arch/Arch.h>

#include <iostream>
#include <fstream>
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

DEFINE_string(intrinsics_filename, "", "Llvm bitcode containing "
              "the intrinsics. "
              "They will be copied to generated module and force-inlined.");
DEFINE_string(basic_blocks_filename, "",
              "Filename of a file containing basic block addresses.");

#pragma clang diagnostic pop

using Memory = std::unordered_map<uint64_t, uint8_t>;

static Memory LoadCode() {
  // mock code
  Memory mem;
  // C7053713000028020000C3
  // 31C064A300000000C3
  std::ifstream f(FLAGS_code_filename, std::ios_base::in | std::ios_base::binary);
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

static std::vector<uint64_t> LoadBasicBlockAddresses() {
  // mock code
  std::vector<uint64_t> res;
  std::ifstream f(FLAGS_basic_blocks_filename, std::ios_base::in);
  while (true) {
    std::uint64_t r;
    f >> r;
    if (f.eof()) break;
    res.push_back(r);
  }
  return res;
}

class SimpleTraceManager : public remill::TraceManager {
 public:
  ~SimpleTraceManager() override = default;

  explicit SimpleTraceManager(Memory &memory_) : memory(memory_) {}

 protected:
  // Called when we have lifted, i.e. defined the contents, of a new trace.
  // The derived class is expected to do something useful with this.
  void SetLiftedTraceDefinition(uint64_t addr,
                                llvm::Function *lifted_func) override {
    traces[addr] = lifted_func;
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
      return trace_it->second;
    } else {
      return nullptr;
    }
  }

  // Get a definition for a lifted trace.
  //
  // NOTE: This is permitted to return a function from an arbitrary module.
  llvm::Function *GetLiftedTraceDefinition(uint64_t addr) override {
    return GetLiftedTraceDeclaration(addr);
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
  Memory &memory;
  std::unordered_map<uint64_t, llvm::Function *> traces;
};

int main(int argc, char** argv) {
  google::SetVersionString(":shrug:");
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  google::SetCommandLineOption("arch", "x86");

  //llvm::cl::

  if (FLAGS_intrinsics_filename.empty()) {
    std::cerr << "Please, specify --intrinsics_filename" << std::endl;
    return EXIT_FAILURE;
  }

  llvm::LLVMContext context;
  auto arch = remill::Arch::GetTargetArch(context);

  std::unique_ptr<llvm::Module> module(remill::LoadArchSemantics(arch));

  //const auto state_ptr_type = remill::StatePointerType(module.get());
  //const auto mem_ptr_type = remill::MemoryPointerType(module.get());

  Memory memory = LoadCode();

  SimpleTraceManager manager(memory);
  remill::IntrinsicTable intrinsics(module);
  remill::InstructionLifter inst_lifter(arch, intrinsics);
  remill::TraceLifter trace_lifter(inst_lifter, manager);

  // Lift all discoverable traces with addresses taken from file
  for (auto addr : LoadBasicBlockAddresses()) {
    trace_lifter.Lift(addr);
  }

  // Optimize the module, but with a particular focus on only the functions
  // that we actually lifted.
  remill::OptimizationGuide guide = {};
  guide.eliminate_dead_stores = true;
  remill::OptimizeModule(arch, module, manager.traces, guide);


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
    remill::MoveFunctionIntoModule(lifted_entry.second, intermediate_module.get());
  }

  auto intrinsics_module
      = remill::LoadModuleFromFile(&context, FLAGS_intrinsics_filename);

  // Here we do some voodoo magic. We link modules with different target
  // triples and data layouts. But this is okay, as there are no pointers
  // inside remill-generated code besides State* and Memory*. They are fine,
  // as they are using fixed-size types, ensuring no padding in-between
  // structure elements, and avoiding arch-specific types like long double.
  intermediate_module->setDataLayout(intrinsics_module->getDataLayout());
  intermediate_module->setTargetTriple(intrinsics_module->getTargetTriple());
  llvm::Linker::linkModules(*intrinsics_module, std::move(intermediate_module));

  guide.slp_vectorize = false;
  guide.loop_vectorize = false;
  guide.eliminate_dead_stores = false;
  guide.verify_input = true;

  remill::OptimizeBareModule(intrinsics_module, guide);

  int ret = EXIT_SUCCESS;

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