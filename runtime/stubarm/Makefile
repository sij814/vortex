include ../common.mk

DESTDIR ?= $(CURDIR)/..

SRC_DIR := $(VORTEX_HOME)/runtime/stubarm

CXX := arm-linux-gnueabihf-g++
CXXFLAGS += -lc -std=c++17 -Wall -Wextra -pedantic -Wfatal-errors
CXXFLAGS += -I$(INC_DIR) -I$(COMMON_DIR) -I$(ROOT_DIR)/hw -I$(SIM_DIR)/common
CXXFLAGS += -fPIC

#LDFLAGS += -shared -pthread -ldl

SRCS := $(SRC_DIR)/vortex.cpp $(SRC_DIR)/utils.cpp
OBJS := $(SRCS:.cpp=.o)

# Debugging
ifdef DEBUG
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O2 -DNDEBUG
endif

PROJECT := libvortex.a

.PHONY: all clean

all: $(DESTDIR)/$(PROJECT)

# 1) Compile .cpp → .o
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 2) Archive .o → .a
$(DESTDIR)/$(PROJECT): $(OBJS)
	@mkdir -p $(DESTDIR)
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJS) $(DESTDIR)/$(PROJECT)
	rm -f $(OBJS) *.o