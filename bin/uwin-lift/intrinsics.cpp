
typedef struct Memory Memory;

#include <cstdint>
#include <cstdlib>

#define HAS_FEATURE_AVX 0
#define HAS_FEATURE_AVX512 0
#define ADDRESS_SIZE_BITS 32
#include <remill/Arch/X86/Runtime/State.h>

struct Memory;



extern "C" {

[[noreturn]]
void uwin_xcute_remill_abort(const char* reason);
Memory *uwin_xcute_remill_dispatch(State &st, uint32_t pc, Memory *mem);
Memory *uwin_xcute_remill_error(State &st, uint32_t pc, Memory *mem);
Memory *uwin_xcute_remill_async_hyper_call(State &st, uint32_t pc, Memory *mem);
Memory *uwin_xcute_remill_sync_hyper_call(State &st, uint32_t pc, Memory *mem);

[[gnu::always_inline]]
Memory *__remill_function_call(State &st, uint32_t pc, Memory *mem) {
  return uwin_xcute_remill_dispatch(st, pc, mem);
}

[[gnu::always_inline]]
Memory *__remill_jump(State &st, uint32_t pc, Memory *mem) {
  return uwin_xcute_remill_dispatch(st, pc, mem);
}

[[gnu::always_inline]]
Memory *__remill_missing_block(State &st, uint32_t pc, Memory *mem) {
  return uwin_xcute_remill_dispatch(st, pc, mem);
}

[[gnu::always_inline]]
Memory *__remill_function_return(State &st, uint32_t pc, Memory *mem) {
  return mem;
}

[[gnu::always_inline]]
Memory *__remill_async_hyper_call(State &st, uint32_t pc, Memory *mem) {
  return uwin_xcute_remill_async_hyper_call(st, pc, mem);
}

[[gnu::always_inline]]
Memory *__remill_sync_hyper_call(State &st, uint32_t pc, Memory *mem) {
  return uwin_xcute_remill_sync_hyper_call(st, pc, mem);
}

[[gnu::always_inline]]
int
__remill_fpu_exception_test_and_clear(int read_mask, int clear_mask) {
  uwin_xcute_remill_abort("__remill_fpu_exception_test_and_clear is not implemented");
}

}

template<typename T>
[[gnu::always_inline]] static inline T* get_addr(Memory* ptr, uint32_t addr) {
    return (T*)((uintptr_t)ptr + addr);
}

// TODO: add an (optional) switch that makes below code ensure all alignment rules are met
// It is not always desirable, as it may have quite a big performance penalty
// But some platforms enforce aligned-only access, so such an ability should exist


extern "C" {
[[gnu::always_inline]]
uint64_t __remill_read_memory_64(Memory *mem, uint32_t addr) {
  auto *ptr = get_addr<uint64_t>(mem, addr);
  return *ptr;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_64(Memory *mem, uint32_t addr, uint64_t value) {
  auto *ptr = get_addr<uint64_t>(mem, addr);
  *ptr = value;
  return mem;
}

[[gnu::always_inline]]
uint32_t __remill_read_memory_32(Memory *mem, uint32_t addr) {
  auto *ptr = get_addr<uint32_t>(mem, addr);
  return *ptr;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_32(Memory *mem, uint32_t addr, uint32_t value) {
  auto *ptr = get_addr<uint32_t>(mem, addr);
  *ptr = value;
  return mem;
}
[[gnu::always_inline]]
uint16_t __remill_read_memory_16(Memory *mem, uint32_t addr) {
  auto *ptr = get_addr<uint16_t>(mem, addr);
  return *ptr;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_16(Memory *mem, uint32_t addr, uint16_t value) {
  auto *ptr = get_addr<uint16_t>(mem, addr);
  *ptr = value;
  return mem;
}
[[gnu::always_inline]]
uint8_t __remill_read_memory_8(Memory *mem, uint32_t addr) {
  auto *ptr = get_addr<uint8_t>(mem, addr);
  return *ptr;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_8(Memory *mem, uint32_t addr, uint8_t value) {
  auto *ptr = get_addr<uint8_t>(mem, addr);
  *ptr = value;
  return mem;
}



[[gnu::always_inline]]
float32_t __remill_read_memory_f32(Memory *, addr_t addr, float32_t val) {
  return *reinterpret_cast<float32_t *>(addr);
}

[[gnu::always_inline]]
float64_t __remill_read_memory_f64(Memory *, addr_t addr, float64_t val) {
  return *reinterpret_cast<float64_t *>(addr);
}

[[gnu::always_inline]]
float64_t __remill_read_memory_f80(Memory *, addr_t addr) {
  uwin_xcute_remill_abort("__remill_read_memory_f80 is not implemented");
  //return static_cast<float64_t>(*reinterpret_cast<long double *>(addr));
}

[[gnu::always_inline]]
Memory *__remill_write_memory_f32(Memory *memory, addr_t addr, float32_t val) {
  *reinterpret_cast<float32_t *>(addr) = val;
  return memory;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_f64(Memory *memory, addr_t addr, float64_t val) {
  *reinterpret_cast<float64_t *>(addr) = val;
  return memory;
}

[[gnu::always_inline]]
Memory *__remill_write_memory_f80(Memory *memory, addr_t addr, float64_t val) {
  uwin_xcute_remill_abort("__remill_write_memory_f80 is not implemented");
  *reinterpret_cast<long double *>(addr) = static_cast<long double>(val);
  return memory;
}


[[gnu::always_inline]]
uint8_t __remill_undefined_8() {
  return 0;
}

[[gnu::always_inline]] uint16_t __remill_undefined_16() {
  return 0;
}

[[gnu::always_inline]] uint32_t __remill_undefined_32() {
  return 0;
}

[[gnu::always_inline]] Memory *__remill_error(State &st, addr_t pc, Memory *mem) {
  return uwin_xcute_remill_error(st, pc, mem);
}

// uwin is single-threaded anyway
[[gnu::always_inline]] Memory *__remill_atomic_begin(Memory* mem) {
  return mem;
}

[[gnu::always_inline]] Memory *__remill_atomic_end(Memory* mem) {
  return mem;
}
}