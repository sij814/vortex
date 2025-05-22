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

#include "simx_device.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <mem.h>
#include "../simx/constants.h"
#include "../simx/processor.h"
#include "../simx/core.h"
#include "VX_types.h"
#include <dram_sim.h>

#include <VX_config.h>

#include <future>
#include <list>
#include <queue>
#include <unordered_map>
#include <util.h>

#ifndef MEM_CLOCK_RATIO
#define MEM_CLOCK_RATIO 1
#endif

#define CACHE_BLOCK_SIZE  64

#define RAM_PAGE_SIZE 4096

#define CPU_GPU_LATENCY 200

using namespace vortex;

///////////////////////////////////////////////////////////////////////////////

class simx_device::Impl {
public:
  Impl()
  : arch_(nullptr)
  , processor_(nullptr)
  , ram_(nullptr)
  , dram_sim_(PLATFORM_MEMORY_NUM_BANKS, PLATFORM_MEMORY_DATA_SIZE, MEM_CLOCK_RATIO)
  , stop_(false)
  {
    // allocate RAM
    arch_ = new Arch(NUM_THREADS, NUM_WARPS, NUM_CORES);
    processor_ = new Processor(*arch_);
    ram_ = new RAM(0, RAM_PAGE_SIZE);

    processor_->attach_ram(ram_);
  }

  ~Impl() {
    stop_ = true;
    if (future_.valid()) {
      future_.wait();
    }
    if (ram_) {
      delete ram_;
    }
  }

  int init() {
    return 0;
  }

  void shutdown() {
    stop_ = true;
    if (future_.valid()) {
      future_.wait();
    }
  }

  int start() {
    // ensure prior run completed
    if (future_.valid()) {
      future_.wait();
    }

    SimPlatform::instance().reset();

    processor_->set_running(true);

    return 0;
  }

  void read_mem(void* data, uint64_t addr, uint64_t size) {
    ram_->enable_acl(false);
    ram_->read(data, addr, size);
    ram_->enable_acl(true);
  }

  void write_mem(const void* data, uint64_t addr, uint64_t size) {
    ram_->enable_acl(false);
    ram_->write(data, addr, size);
    ram_->enable_acl(true);
  }

  int dcr_write(uint32_t addr, uint32_t value) {
    if (future_.valid())
    {
      future_.wait();
    }
  
    processor_->dcr_write(addr, value);

    return 0;
  }

  void proc_tick() {
    // tick run
    processor_->run();
  }

  bool get_running() {
    return processor_->get_running();
  }
  
  Arch* arch_;
  Processor* processor_;
  RAM* ram_;
  DramSim dram_sim_;

  std::future<void> future_;
  bool stop_;

  std::mutex mutex_;
};

///////////////////////////////////////////////////////////////////////////////

simx_device::simx_device()
  : impl_(new Impl())
{}

simx_device::~simx_device() {
  delete impl_;
}

int simx_device::init() {
  return impl_->init();
}

void simx_device::shutdown() {
  impl_->shutdown();
}

int simx_device::start() {
  return impl_->start();
}

void simx_device::write_mem(const void* data, uint64_t addr, uint64_t size) {
  impl_->write_mem(data, addr, size);
}

void simx_device::read_mem(void* data, uint64_t addr, uint64_t size) {
  impl_->read_mem(data, addr, size);
}

int simx_device::dcr_write(uint32_t addr, uint32_t value) {
  return impl_->dcr_write(addr, value);
}

void simx_device::proc_tick() {
  impl_->proc_tick();
}

bool simx_device::get_running() {
  return impl_->get_running();
}
