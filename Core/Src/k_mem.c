#include "common.h"
#include "k_mem.h"
#include <stdint.h>

int k_mem_init(void) {
	__asm volatile("svc %[n]" :: [n] "I"(SVC_MEMORY_INIT));
}

void* k_mem_alloc(size_t size) {
    register size_t r0 __asm("r0") = size;
    __asm volatile(
        "svc %[svc]"
        : "+r"(r0)
        : [svc] "I"(SVC_MEMORY_ALLOC)
        : "memory");
    return (void*)r0;
}

int k_mem_dealloc(void* ptr) {
    register void* r0 __asm("r0") = ptr;
    __asm volatile(
        "svc %[svc]"
        : "+r"(r0)
        : [svc] "I"(SVC_MEMORY_DEALLOC)
        : "memory");
    return (int)r0;
}

int k_mem_count_extfrag(size_t size) {
    register size_t r0 __asm("r0") = size;
    __asm volatile(
        "svc %[svc]"
        : "+r"(r0)
        : [svc] "I"(SVC_MEMORY_UTILITY)
        : "memory");
    return (int)r0;
}
