include ../common.mk

DESTDIR ?= $(CURDIR)

SRC_DIR := $(VORTEX_HOME)/sim/opaesimx
AFU_DIR := $(RTL_DIR)/afu/opae

CXXFLAGS += -std=c++17 -Wall -Wextra -Wfatal-errors -Wno-array-bounds
CXXFLAGS += -fPIC -Wno-maybe-uninitialized
CXXFLAGS += -I$(SRC_DIR) -I$(ROOT_DIR)/hw -I$(COMMON_DIR) -I$(DESTDIR)
CXXFLAGS += -I/$(THIRD_PARTY_DIR)/softfloat/source/include
CXXFLAGS += -I$(THIRD_PARTY_DIR)/ramulator/ext/spdlog/include
CXXFLAGS += -I$(THIRD_PARTY_DIR)/ramulator/ext/yaml-cpp/include
CXXFLAGS += -I$(THIRD_PARTY_DIR)/ramulator/src
CXXFLAGS += -DXLEN_$(XLEN)

LDFLAGS += -shared $(THIRD_PARTY_DIR)/softfloat/build/Linux-x86_64-GCC/softfloat.a
LDFLAGS += -Wl,-rpath,$(THIRD_PARTY_DIR)/ramulator -L$(THIRD_PARTY_DIR)/ramulator -lramulator -pthread

SRCS = $(COMMON_DIR)/util.cpp $(COMMON_DIR)/mem.cpp $(COMMON_DIR)/softfloat_ext.cpp $(COMMON_DIR)/rvfloats.cpp $(COMMON_DIR)/dram_sim.cpp
#SRCS += $(DPI_DIR)/util_dpi.cpp $(DPI_DIR)/float_dpi.cpp
SRCS += $(SRC_DIR)/fpga.cpp $(SRC_DIR)/opae_simx.cpp

CXXFLAGS += $(CONFIGS)

# Enable scope analyzer
ifdef SCOPE
	CXXFLAGS += -DSCOPE
	SCOPE_JSON = $(DESTDIR)/scope.json
endif

# Enable perf counters
ifdef PERF
	CXXFLAGS += -DPERF_ENABLE
endif

# use our OPAE shim
CXXFLAGS += -DNOPAE

PROJECT := libopaesimx-c-sim.so

all: $(DESTDIR)/$(PROJECT)

$(DESTDIR)/$(PROJECT): $(SRCS)
	$(CXX) $(CXXFLAGS) -DSTARTUP_ADDR=0x80000000 $^ $(LDFLAGS) -o $@

$(DESTDIR)/lib$(PROJECT).so: $(SRCS)
	$(CXX) $(CXXFLAGS) $^ -shared $(LDFLAGS) -o $@

.depend: $(SRCS)
	$(CXX) $(CXXFLAGS) -MM $^ > .depend;

clean-lib:
	rm -f $(DESTDIR)/lib$(PROJECT).so

clean-exe:
	rm -f $(DESTDIR)/$(PROJECT)

clean: clean-lib clean-exe
