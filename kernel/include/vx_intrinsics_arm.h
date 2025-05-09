// Copyright © 2019-2023
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef __VX_INTRINSICS_ARM_H__
#define __VX_INTRINSICS_ARM_H__

#include <stddef.h>
#include <stdint.h>
#include <VX_types.h>

// ARM CP15 registers for system control (equivalent to RISC-V CSRs)
#define ARM_CP15_THREAD_ID       "c0, c0, 5"   // MPIDR (Multiprocessor Affinity Register)
#define ARM_CP15_ACTIVE_THREADS  "c0, c1, 0"
#define ARM_CP15_ACTIVE_WARPS    "c0, c1, 1"
#define ARM_CP15_NUM_THREADS     "c0, c2, 0"
#define ARM_CP15_NUM_WARPS       "c0, c2, 1"
#define ARM_CP15_NUM_CORES       "c0, c2, 2"
#define ARM_CP15_WARP_ID         "c0, c3, 0"
#define ARM_CP15_CORE_ID         "c0, c3, 1"

#if defined(__clang__)
#define __UNIFORM__   __attribute__((annotate("vortex.uniform")))
#else
#define __UNIFORM__
#endif

#ifdef __cplusplus
extern "C" {
#endif

// CP15 Register Access
#define cp15_read(reg) ({                       \
    uint32_t __r;                                \
    __asm__ __volatile__ ("mrc p15, 0, %0, " reg    \
                      : "=r" (__r));             \
    __r;                                         \
})

#define cp15_write(reg, val) ({                 \
    uint32_t __v = (uint32_t)(val);              \
    __asm__ __volatile__ ("mcr p15, 0, %0, " reg    \
                      :: "r" (__v));             \
})

// Thread Control
inline void vx_tmc(uint32_t thread_mask) {
    // ARM implementation using CP15 or custom hardware
    __asm__ __volatile__ ("mcr p15, 0, %0, c1, c0, 0" :: "r" (thread_mask));
}

inline void vx_tmc_zero() {
    vx_tmc(0);
}

inline void vx_tmc_one() {
    vx_tmc(1);
}

// Predication
inline void vx_pred(uint32_t condition, uint32_t thread_mask) {
    // Custom predication implementation
    __asm__ __volatile__ (
        "and %0, %0, %1\n\t"
        "mcr p15, 0, %0, c1, c1, 0" 
        :: "r" (condition), "r" (thread_mask)
    );
}

// Warp Control
inline void vx_wspawn(uint32_t num_warps, void (*func_ptr)()) {
    // Custom warp spawn implementation
    __asm__ __volatile__ (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mcr p15, 0, r0, c1, c2, 0"
        :: "r" (num_warps), "r" (func_ptr)
        : "r0", "r1"
    );
}

// Synchronization
inline void vx_barrier(uint32_t barrier_id, uint32_t num_warps) {
    // DMB instruction for ARM memory barriers
    __asm__ __volatile__ (
        "dmb sy\n\t"
        "mcr p15, 0, %0, c7, c10, 5"
        :: "r" ((barrier_id << 16) | num_warps)
    );
}

// Thread Information
inline uint32_t vx_thread_id() {
    return cp15_read(ARM_CP15_THREAD_ID) & 0xFF;
}

inline uint32_t vx_warp_id() {
    return cp15_read(ARM_CP15_WARP_ID);
}

inline uint32_t vx_core_id() {
    return cp15_read(ARM_CP15_CORE_ID);
}

inline uint32_t vx_active_threads() {
    return cp15_read(ARM_CP15_ACTIVE_THREADS);
}

inline uint32_t vx_active_warps() {
    return cp15_read(ARM_CP15_ACTIVE_WARPS);
}

inline uint32_t vx_num_threads() {
    return cp15_read(ARM_CP15_NUM_THREADS);
}

inline uint32_t vx_num_warps() {
    return cp15_read(ARM_CP15_NUM_WARPS);
}

inline uint32_t vx_num_cores() {
    return cp15_read(ARM_CP15_NUM_CORES);
}

inline uint32_t vx_hart_id() {
    return cp15_read(ARM_CP15_THREAD_ID);
}

inline void vx_fence() {
    __asm__ __volatile__ ("dmb sy");
}

// Matrix Operations (ARM NEON equivalent)
inline void vx_matrix_load(uint32_t dest, uint32_t addr) {
    // TODO: Implement using NEON VLDM/VSTM
    __asm__ __volatile__ ("pld [%0]" :: "r" (addr));
}

inline void vx_matrix_store(uint32_t addr) {
    // TODO: Implement using NEON VSTM
    __asm__ __volatile__ ("dmb st");
}

inline void vx_matrix_mul() {
    // TODO: Implement using NEON VMLA
    __asm__ __volatile__ ("" ::: "memory");
}

#ifdef __cplusplus
}
#endif

#endif // __VX_INTRINSICS_ARM_H__