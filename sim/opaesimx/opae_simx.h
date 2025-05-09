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

#pragma once

#include <stdint.h>

namespace vortex {

class opae_simx {
public:

  opae_simx();
  virtual ~opae_simx();

  int init();

  void shutdown();

  int start();

  int start(uint64_t krnl_addr, uint64_t args_addr);

  int prepare_buffer(uint64_t len, void **buf_addr, uint64_t *wsid, int flags);

  void release_buffer(uint64_t wsid);

  void get_io_address(uint64_t wsid, uint64_t *ioaddr);

  void write_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t value, uint64_t size);

  void read_mmio64(uint32_t mmio_num, uint64_t offset, uint64_t *value, uint64_t size);

  void write_mem(const void* data, uint64_t addr, uint64_t size);

  void read_mem(void* data, uint64_t addr, uint64_t size);

  void proc_tick();

  bool get_running();
private:
  class Impl;
  Impl* impl_;
};

}