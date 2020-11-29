
typedef struct Memory Memory;

#include <stdint.h>
#include <cstdlib>

#define HAS_FEATURE_AVX 0
#define HAS_FEATURE_AVX512 0
#define ADDRESS_SIZE_BITS 32
#include <remill/Arch/X86/Runtime/State.h>

struct Memory;

extern "C" {
    Memory* uwin_remill_dispatch(State* st, uint32_t pc, Memory* mem);

    __attribute__((always_inline))
    Memory* __remill_function_call(State* st, uint32_t pc, Memory* mem) {
        return uwin_remill_dispatch(st, pc, mem);
    }

    __attribute__((always_inline))
    Memory* __remill_function_return(State* st, uint32_t pc, Memory* mem) {
        return mem;
    }
}

template<typename T>
__attribute__((always_inline))
static inline T* get_addr(Memory* ptr, uint32_t addr) {
    return (T*)((uintptr_t)ptr + addr);
}

extern "C" {
    __attribute__((always_inline))
    uint32_t __remill_read_memory_32(Memory* mem, uint32_t addr) {
        auto* ptr = get_addr<uint32_t>(mem, addr); // TODO: alignment?
        return *ptr;
    }

    __attribute__((always_inline))
    Memory* __remill_write_memory_32(Memory* mem, uint32_t addr, uint32_t value) {
        auto* ptr = get_addr<uint32_t>(mem, addr); // TODO: alignment?
        *ptr = value;
        return mem;
    }
}
