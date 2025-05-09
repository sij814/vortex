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

#include "opae_simx.h"

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

class opae_simx::Impl {
public:
  Impl()
  : arch_(nullptr)
  , processor_(nullptr)
  , ram_(nullptr)
  , dram_sim_(PLATFORM_MEMORY_NUM_BANKS, PLATFORM_MEMORY_DATA_SIZE, MEM_CLOCK_RATIO)
  , stop_(false)
  , host_buffer_ids_(0)
  {}

  ~Impl() {
    stop_ = true;
    if (future_.valid()) {
      future_.wait();
    }
    for (auto& buffer : host_buffers_) {
      aligned_free(buffer.second.data);
    }
    if (ram_) {
      delete ram_;
    }
  }

  int init() {
    // allocate RAM
    arch_ = new Arch(NUM_THREADS, NUM_WARPS, NUM_CORES);
    processor_ = new Processor(*arch_);
    ram_ = new RAM(0, RAM_PAGE_SIZE);

    processor_->attach_ram(ram_);

    // reset the device
    this->reset();

    /*
    const char* program = "/home/sij814/gem5-vortex/ext/vortex/build/tests/kernel/conform/conform.bin";
    //const char* program = "/home/sij814/gem5-vortex/ext/vortex/build/tests/regression/vecaddx/kernel.vxbin";

    {
      std::string program_ext(fileExtension(program));
      if (program_ext == "bin" || program_ext == "vxbin") {
        ram_->loadBinImage(program, startup_addr);
      } else if (program_ext == "hex") {
        ram_->loadHexImage(program);
      } else {
        std::cout << "*** error: only *.bin or *.hex images supported." << std::endl;
        return -1;
      }
    }
    */

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
    if (future_.valid())
    {
      future_.wait();
    }

    // setup base DCRs

    const uint64_t startup_addr(STARTUP_ADDR);

    processor_->dcr_write(VX_DCR_BASE_STARTUP_ADDR0, startup_addr & 0xffffffff);
  #if (XLEN == 64)
    processor_->dcr_write(VX_DCR_BASE_STARTUP_ADDR1, startup_addr >> 32);
  #endif
    processor_->dcr_write(VX_DCR_BASE_MPM_CLASS, 0);

    SimPlatform::instance().reset();

    processor_->set_running(true);

    return 0;
  }

  int start(uint64_t krnl_addr, uint64_t args_addr) {
    // ensure prior run completed
    if (future_.valid())
    {
      future_.wait();
    }

    // setup base DCRs
    processor_->dcr_write(VX_DCR_BASE_STARTUP_ADDR0, krnl_addr & 0xffffffff);
    processor_->dcr_write(VX_DCR_BASE_STARTUP_ADDR1, krnl_addr >> 32);
    processor_->dcr_write(VX_DCR_BASE_STARTUP_ARG0, args_addr & 0xffffffff);
    processor_->dcr_write(VX_DCR_BASE_STARTUP_ARG1, args_addr >> 32);

    SimPlatform::instance().reset();

    processor_->set_running(true);

    return 0;
  }

  int prepare_buffer(uint64_t len, void **buf_addr, uint64_t *wsid, int flags) {
    auto alloc = aligned_malloc(len, CACHE_BLOCK_SIZE);
    if (alloc == NULL)
      return -1;
    // set uninitialized data to "baadf00d"
    for (uint32_t i = 0; i < len; ++i) {
      ((uint8_t*)alloc)[i] = (0xbaadf00d >> ((i & 0x3) * 8)) & 0xff;
    }
    host_buffer_t buffer;
    buffer.data   = (uint64_t*)alloc;
    buffer.size   = len;
    buffer.ioaddr = uintptr_t(alloc);
    auto buffer_id = host_buffer_ids_++;
    host_buffers_.emplace(buffer_id, buffer);
    *buf_addr = alloc;
    *wsid = buffer_id;
    return 0;
  }

  void release_buffer(uint64_t wsid) {
    auto it = host_buffers_.find(wsid);
    if (it != host_buffers_.end()) {
      aligned_free(it->second.data);
      host_buffers_.erase(it);
    }
  }

  void get_io_address(uint64_t wsid, uint64_t *ioaddr) {
    *ioaddr = host_buffers_[wsid].ioaddr;
  }

  void read_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t *value, uint64_t size) {
    std::lock_guard<std::mutex> guard(mutex_);

    //std::cout << "read 1" << std::endl;

    // simulate CPU-GPU latency
    for (uint32_t i = 0; i < CPU_GPU_LATENCY; ++i) {
      this->tick();
    }

    //std::cout << "read 2" << std::endl;

    // simulate mmio request
    this->tick();

    //std::cout << "read 3" << std::endl;

    ram_->read(value, offset, size);

    //std::cout << "read 4" << std::endl;
  }

  void write_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t value, uint64_t size) {
    std::lock_guard<std::mutex> guard(mutex_);

    //std::cout << "write 1" << std::endl;

    // simulate CPU-GPU latency
    for (uint32_t i = 0; i < CPU_GPU_LATENCY; ++i) {
      this->tick();
    }

    //std::cout << "write 2" << std::endl;

    // simulate mmio request
    this->tick();

    //std::cout << "write 3" << std::endl;

    ram_->write(&value, offset, size);

    //std::cout << "write 4" << std::endl;
  }

  void read_mem(void* data, uint64_t addr, uint64_t size) {
    ram_->read(data, addr, size);
  }

  void write_mem(const void* data, uint64_t addr, uint64_t size) {
    ram_->write(data, addr, size);
  }

  void proc_tick() {
    // tick run
    processor_->run();
  }

  bool get_running() {
    return processor_->get_running();
  }

private:

  void reset() {
    for (auto& reqs : pending_mem_reqs_) {
      reqs.clear();
    }

    {
      std::queue<mem_req_t*> empty;
      std::swap(dram_queue_, empty);
    }
  }

  void tick() {
    if (!dram_queue_.empty()) {
      auto mem_req = dram_queue_.front();
      dram_sim_.send_request(mem_req->addr, mem_req->write, [](void* arg) {
        auto orig_req = reinterpret_cast<mem_req_t*>(arg);
        if (orig_req->ready) {
          delete orig_req;
        } else {
          orig_req->ready = true;
        }
      }, mem_req);
      dram_queue_.pop();
    }

    dram_sim_.tick();

  #ifndef NDEBUG
    fflush(stdout);
  #endif
  }

  typedef struct {
    std::array<uint8_t, PLATFORM_MEMORY_DATA_SIZE> data;
    uint32_t addr;
    uint32_t bank_id;
    bool write;
    bool ready;
  } mem_req_t;

  typedef struct {
    uint64_t* data;
    size_t    size;
    uint64_t  ioaddr;
  } host_buffer_t;

  Arch* arch_;
  Processor* processor_;
  RAM* ram_;
  DramSim dram_sim_;

  std::future<void> future_;
  bool stop_;

  std::unordered_map<int64_t, host_buffer_t> host_buffers_;
  uint64_t host_buffer_ids_;

  std::list<mem_req_t*> pending_mem_reqs_[PLATFORM_MEMORY_NUM_BANKS];

  std::mutex mutex_;

  std::queue<mem_req_t*> dram_queue_;
};

///////////////////////////////////////////////////////////////////////////////

opae_simx::opae_simx()
  : impl_(new Impl())
{}

opae_simx::~opae_simx() {
  delete impl_;
}

int opae_simx::init() {
  return impl_->init();
}

void opae_simx::shutdown() {
  impl_->shutdown();
}

int opae_simx::start() {
  return impl_->start();
}

int opae_simx::start(uint64_t krnl_addr, uint64_t args_addr) {
  return impl_->start(krnl_addr, args_addr);
}

int opae_simx::prepare_buffer(uint64_t len, void **buf_addr, uint64_t *wsid, int flags) {
  return impl_->prepare_buffer(len, buf_addr, wsid, flags);
}

void opae_simx::release_buffer(uint64_t wsid) {
  impl_->release_buffer(wsid);
}

void opae_simx::get_io_address(uint64_t wsid, uint64_t *ioaddr) {
  impl_->get_io_address(wsid, ioaddr);
}

void opae_simx::write_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t value, uint64_t size) {
  impl_->write_mmio64(mmio_num, offset, value, size);
}

void opae_simx::read_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t *value, uint64_t size) {
  impl_->read_mmio64(mmio_num, offset, value, size);
}

void opae_simx::write_mem(const void* data, uint64_t addr, uint64_t size) {
  impl_->write_mem(data, addr, size);
}

void opae_simx::read_mem(void* data, uint64_t addr, uint64_t size) {
  impl_->read_mem(data, addr, size);
}

void opae_simx::proc_tick() {
  impl_->proc_tick();
}

bool opae_simx::get_running() {
  return impl_->get_running();
}
