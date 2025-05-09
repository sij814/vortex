include ../common.mk

DESTDIR ?= $(CURDIR)/..

SRC_DIR := $(VORTEX_HOME)/runtime/stubarm
OPAE_DIR := $(VORTEX_HOME)/runtime/opaesimx
SIM_COMMON_DIR := $(SIM_DIR)/common
SIM_SIMX_DIR := $(SIM_DIR)/simx

CXX := arm-linux-gnueabihf-g++
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -Wfatal-errors
CXXFLAGS += -I$(INC_DIR) -I$(COMMON_DIR) -I$(ROOT_DIR)/hw -I$(SIM_COMMON_DIR) -I$(OPAE_DIR) -I$(SIM_DIR) -I$(VORTEX_HOME)/runtime
CXXFLAGS += -fPIC

LDFLAGS += -static -pthread

SRCS := $(SIM_DIR)/opaesimx/opae_simx.cpp $(SIM_DIR)/opaesimx/fpga.cpp $(SRC_DIR)/driver.cpp $(SRC_DIR)/vortex.cpp $(SRC_DIR)/utils.cpp
SRCS += $(SIM_COMMON_DIR)/util.cpp $(SIM_COMMON_DIR)/mem.cpp $(SIM_COMMON_DIR)/softfloat_ext.cpp $(SIM_COMMON_DIR)/rvfloats.cpp $(SIM_COMMON_DIR)/dram_sim.cpp
SRCS += $(SIM_SIMX_DIR)/processor.cpp $(SIM_SIMX_DIR)/cluster.cpp $(SIM_SIMX_DIR)/socket.cpp $(SIM_SIMX_DIR)/core.cpp $(SIM_SIMX_DIR)/emulator.cpp $(SIM_SIMX_DIR)/decode.cpp $(SIM_SIMX_DIR)/execute.cpp $(SIM_SIMX_DIR)/func_unit.cpp $(SIM_SIMX_DIR)/cache_sim.cpp $(SIM_SIMX_DIR)/mem_sim.cpp $(SIM_SIMX_DIR)/local_mem.cpp $(SIM_SIMX_DIR)/mem_coalescer.cpp $(SIM_SIMX_DIR)/dcrs.cpp $(SIM_SIMX_DIR)/types.cpp

# Debugging
ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O2 -DNDEBUG
endif

PROJECT := libvortexarm.a

all: $(DESTDIR)/$(PROJECT)

driver: $(DESTDIR)/libvortex-opaesimx.so

$(DESTDIR)/$(PROJECT): $(SRCS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(DESTDIR)/$(PROJECT)

.PHONY: all clean