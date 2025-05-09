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
#include "common.h"

.type vx_putchar, %function
.global vx_putchar
vx_putchar:
    /* Get hardware thread ID (Core ID) */
    mrc p15, 0, r1, c0, c0, 5   @ Read MPIDR (Multiprocessor Affinity Register)
    and r1, r1, #0xFF           @ Extract CPU ID (Affinity 0)

    /* Calculate I/O address offset */
    and r1, r1, #(IO_COUT_SIZE-1) @ Mask with buffer size -1
    
    /* Get base I/O address */
    ldr r2, =IO_COUT_ADDR        @ Load cout base address
    
    /* Store character to I/O location */
    add r2, r2, r1              @ Calculate final address
    strb r0, [r2]               @ Store byte to I/O location
    
    bx lr                       @ Return to caller