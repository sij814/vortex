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

#define RISCV_CUSTOM0   0x0B

.type vx_serial, %function
.global vx_serial
vx_serial:
    /* Prologue - Save registers */
    push    {r4-r9, lr}         @ Save callee-saved registers and LR (7 regs * 4B = 28B)
    
    /* Argument handling */
    mov     r8, r0              @ r8 (s4) = callback
    mov     r7, r1              @ r7 (s3) = arg
    
    /* Get thread information */
    @ Replace with ARM-specific CSR access or system calls
    bl      get_num_threads     @ Custom function to get num_threads
    mov     r6, r0              @ r6 (s2) = num_threads
    bl      get_thread_id        @ Custom function to get thread_id
    mov     r5, r0              @ r5 (s1) = thread_id
    
    /* Initialize loop counter */
    mov     r4, #0              @ r4 (s0) = index = 0

1:  /* label_loop */
    /* Check (index != thread_id) */
    subs    r2, r4, r5          @ r2 = index - thread_id
    cmp     r2, #0
    beq     2f                  @ if (index == thread_id), jump to callback
    
    /* ARM equivalent of .insn split */
    @ --- Begin Split Operation ---
    @ Custom synchronization logic here
    dmb     sy                  @ Example: Data Memory Barrier
    @ --- End Split Operation ---
    b       3f

2:  /* label_join (callback path) */
    /* Execute callback */
    mov     r0, r7              @ arg = s3 (r7)
    blx     r8                  @ Call s4 (callback)
    
    /* ARM equivalent of .insn join */
    @ --- Begin Join Operation ---
    dmb     sy                  @ Example: Data Memory Barrier
    @ --- End Join Operation ---

3:  /* Continue loop */
    add     r4, r4, #1          @ index++
    cmp     r4, r6              @ Compare index vs num_threads
    blt     1b                  @ Loop if index < num_threads

    /* Epilogue - Restore registers */
    pop     {r4-r9, pc}         @ Restore registers and return

