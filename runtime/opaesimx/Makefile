ROOT_DIR := $(realpath ../..)
include ../common.mk

TARGET ?= opaesimx

DESTDIR ?= $(CURDIR)/..

SYN_DIR := $(HW_DIR)/syn/altera/opae

SRC_DIR := $(VORTEX_HOME)/runtime/opaesimx

CXX := arm-linux-gnueabihf-g++
CXXFLAGS += -std=c++17 -Wall -Wextra -pedantic -Wfatal-errors
CXXFLAGS += -I$(INC_DIR) -I$(COMMON_DIR) -I$(ROOT_DIR)/hw -I$(DESTDIR) -I$(SIM_DIR)/common
CXXFLAGS += -DXLEN_$(XLEN)

# Position independent code
CXXFLAGS += -fPIC

# Add external configuration
CXXFLAGS += $(CONFIGS)

SRCS = $(SRC_DIR)/vortex.cpp $(SRC_DIR)/driver.cpp
OBJS := $(SRCS:.cpp=.o)

# set up target types
ifeq ($(TARGET), opaesimx)
	BUILD_DEPS = $(DESTDIR)/libopaesimx-c-sim.a
	CXXFLAGS += -DOPAESIMX -I$(SIM_DIR)/opaesimx
else
	BUILD_DEPS = $(ROOT_DIR)/hw/syn/altera/opae/vortex_afu.h
	CXXFLAGS += -I$(SYN_DIR) -I$(ROOT_DIR)/hw/syn/altera/opae
endif

# Debugging
ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O2 -DNDEBUG
endif

# Enable scope logic analyzer
ifdef SCOPE
	CXXFLAGS += -DSCOPE
	SRCS += $(COMMON_DIR)/scope.cpp
endif

PROJECT := libvortex-opaesimx.a

all: $(DESTDIR)/$(PROJECT)

driver: $(DESTDIR)/libopaesimx-c-sim.a

$(DESTDIR)/libopaesimx-c-sim.a:
	DESTDIR=$(DESTDIR) $(MAKE) -C $(ROOT_DIR)/sim/opaesimx $(DESTDIR)/libopaesimx-c-sim.a

#$(DESTDIR)/$(PROJECT): $(SRCS) $(BUILD_DEPS)
#	$(CXX) $(CXXFLAGS) $(SRCS) $(LDFLAGS) -o $@

# 1) compile .cpp -> .o
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 2) archive .o -> .a
$(DESTDIR)/$(PROJECT): $(BUILD_DEPS) $(OBJS)
	@mkdir -p $(DESTDIR)
	$(AR) rcs $@ $(OBJS)

clean-driver:
	DESTDIR=$(DESTDIR) $(MAKE) -C $(ROOT_DIR)/sim/opaesimx clean

clean-runtime:
	rm -f $(DESTDIR)/$(PROJECT)
	rm -f $(OBJS) *.o

clean: clean-driver clean-runtime

.PHONY: all driver clean-driver clean-runtime clean