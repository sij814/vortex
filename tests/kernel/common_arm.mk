ROOT_DIR := $(realpath ../../..)

ARM_TOOLCHAIN_PATH := /usr/bin
ARM_PREFIX := arm-none-eabi

# Set ARM-specific architecture flags.
CFLAGS += -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard

STARTUP_ADDR ?= 0x80000000

VORTEX_KN_PATH ?= $(ROOT_DIR)/kernel

# Optionally, LLVM_CFLAGS can be omitted or adjusted for ARM.
#LLVM_CFLAGS += --sysroot=$(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)
#LLVM_CFLAGS += --gcc-toolchain=$(ARM_TOOLCHAIN_PATH)
#LLVM_CFLAGS += -Xclang -target-feature -Xclang +vortex -mllvm -vortex-branch-divergence=0

# Use ARM GCC toolchain instead of RISC-V.
CC  = $(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)-gcc
CXX = $(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)-g++
AR  = $(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)-gcc-ar
DP  = $(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)-objdump
CP  = $(ARM_TOOLCHAIN_PATH)/$(ARM_PREFIX)-objcopy

CFLAGS += -O3 -mcmodel=medany -fno-exceptions -nostartfiles -fdata-sections -ffunction-sections
CFLAGS += -I$(VORTEX_HOME)/kernel/include -I$(ROOT_DIR)/hw
CFLAGS += -DXLEN_$(XLEN) -DNDEBUG

# Update library search to use ARM libraries.
#LIBC_LIB += -L$(LIBC_VORTEX)/lib -lm -lc
# Replace the RISC-V builtins library with an ARM version.
#LIBC_LIB += $(LIBCRT_VORTEX)/lib/baremetal/libclang_rt.builtins-arm.a

# Use an ARM-specific linker script.
LDFLAGS += -Wl,-Bstatic,--gc-sections,-T,$(VORTEX_HOME)/kernel/scripts/linkarm.ld,--defsym=STARTUP_ADDR=$(STARTUP_ADDR) $(VORTEX_KN_PATH)/libvortex.a #$(LIBC_LIB)

all: $(PROJECT).elf $(PROJECT).bin $(PROJECT).dump

$(PROJECT).dump: $(PROJECT).elf
	$(DP) -D $< > $@

$(PROJECT).bin: $(PROJECT).elf
	$(CP) -O binary $< $@

$(PROJECT).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

run-rtlsim: $(PROJECT).bin
	$(ROOT_DIR)/sim/rtlsim/rtlsim $(PROJECT).bin

run-simx: $(PROJECT).bin
	$(ROOT_DIR)/sim/simx/simx $(PROJECT).bin

.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $^ > .depend;

clean:
	rm -rf *.elf *.bin *.dump *.log .depend
