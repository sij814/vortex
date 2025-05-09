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

#include <VX_config.h>
#include <VX_types.h>
#include <newlib.h>
#include "common.h"

.section .init, "ax"
.global _start
.type   _start, %function
_start:
    /* Initialize per-thread registers */
    bl      get_num_warps         @ r0 = VX_CSR_NUM_WARPS equivalent
    ldr     r1, =init_regs_all
    bl      wspawn_arm            @ Custom thread spawn implementation
    mov     r0, #-1
    bl      tmc_arm               @ Thread Mask Control implementation
    bl      init_regs
    mov     r0, #1
    bl      tmc_arm

    /* Initialize TLS for all warps */
    bl      get_num_warps
    ldr     r1, =init_tls_all
    bl      wspawn_arm
    mov     r0, #-1
    bl      tmc_arm
    bl      __init_tls
    mov     r0, #1
    bl      tmc_arm

    /* Clear BSS segment */
    ldr     r0, =_edata
    ldr     r2, =_end
    sub     r1, r2, r0
    mov     r2, #0
    bl      memset

#ifdef HAVE_INITFINI_ARRAY
    /* Run global initialization functions */
    bl      __libc_init_array
#endif

    /* Call main program routine */
    bl      main

    /* Call exit routine */
    b       exit

.size _start, .-_start

.section .text
.type _Exit, %function
.global _Exit
_Exit:
    bl      vx_perf_dump
    ldr     r1, =IO_MPM_EXITCODE
    str     r0, [r1]
    dmb     sy                  @ Data memory barrier
    mov     r0, #0
    bl      tmc_arm             @ Terminate all threads

.section .text
.type init_regs, %function
.local init_regs
init_regs:
    /* Set global pointer register */
    ldr     r12, =__global_pointer

    /* Set stack pointer register */
    ldr     sp, =STACK_BASE_ADDR
    bl      get_hart_id          @ Get hardware thread ID
    lsl     r1, r0, #STACK_LOG2_SIZE
    sub     sp, sp, r1

    /* Set thread pointer register */
    ldr     r1, =__tbss_size
    mul     r0, r0, r1
    ldr     r2, =_end
    add     r2, r2, r0
    mov     r9, r2              @ ARM TLS register (typically r9)
    bx      lr

.section .text
.type init_regs_all, %function
.local init_regs_all
init_regs_all:
    mov     r0, #-1
    bl      tmc_arm
    bl      init_regs
    mov     r0, #0
    bl      tmc_arm
    bx      lr

.section .text
.type init_tls_all, %function
.local init_tls_all
init_tls_all:
    mov     r0, #-1
    bl      tmc_arm
    bl      __init_tls
    mov     r0, #0
    bl      tmc_arm
    bx      lr

.section .data
.global __dso_handle
.weak __dso_handle
__dso_handle:
    .long   0

/* Custom ARM implementations of RISC-V CSR operations */
.type tmc_arm, %function
tmc_arm:
    @ Implement thread mask control logic here
    @ r0 = mask value
    bx      lr

.type wspawn_arm, %function
wspawn_arm:
    @ Implement thread spawn logic here
    @ r0 = num_warps, r1 = init function
    bx      lr

.type get_num_warps, %function
get_num_warps:
    @ Implement num_warps retrieval
    mrc     p15, 0, r0, c0, c0, 5  @ Example using CP15
    and     r0, r0, #0xFF
    bx      lr

.type get_hart_id, %function
get_hart_id:
    @ Implement hardware thread ID retrieval
    mrc     p15, 0, r0, c0, c0, 5  @ MPIDR register
    and     r0, r0, #0xFF
    bx      lr