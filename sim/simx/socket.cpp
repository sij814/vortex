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

#include "socket.h"
#include "cluster.h"

using namespace vortex;

Socket::Socket(const SimContext& ctx,
                uint32_t socket_id,
                Cluster* cluster,
                const Arch &arch,
                const DCRS &dcrs)
  : SimObject(ctx, "socket")
  , icache_mem_req_port(this)
  , icache_mem_rsp_port(this)
  , dcache_mem_req_ports(DCACHE_NUM_BANKS, this)
  , dcache_mem_rsp_ports(DCACHE_NUM_BANKS, this)
  , l1_mem_req_ports(L1_NUM_MEM_PORTS, this)
  , l1_mem_rsp_ports(L1_NUM_MEM_PORTS, this)
  , socket_id_(socket_id)
  , cluster_(cluster)
  , cores_(arch.socket_size())
{
  auto cores_per_socket = cores_.size();

  char sname[100];
  snprintf(sname, 100, "socket%d-icaches", socket_id);
  icaches_ = CacheCluster::Create(sname, cores_per_socket, NUM_ICACHES, 1, CacheSim::Config{
    !ICACHE_ENABLED,
    log2ceil(ICACHE_SIZE),  // C
    log2ceil(L1_LINE_SIZE), // L
    log2ceil(sizeof(uint32_t)), // W
    log2ceil(ICACHE_NUM_WAYS), // A
    1,                      // B
    XLEN,                   // address bits
    1,                      // number of ports
    1,                      // number of inputs
    1,                      // number of outputs
    false,                  // write-back
    false,                  // write response
    (uint8_t)arch.num_warps(), // mshr size
    2,                      // pipeline latency
  });

  /*
  icaches_->MemReqPorts.at(0).bind(&icache_mem_req_port);
  icache_mem_rsp_port.bind(&icaches_->MemRspPorts.at(0));
  */

  snprintf(sname, 100, "socket%d-dcaches", socket_id);
  dcaches_ = CacheCluster::Create(sname, cores_per_socket, NUM_DCACHES, DCACHE_NUM_REQS, CacheSim::Config{
    !DCACHE_ENABLED,
    log2ceil(DCACHE_SIZE),  // C
    log2ceil(L1_LINE_SIZE), // L
    log2ceil(DCACHE_WORD_SIZE), // W
    log2ceil(DCACHE_NUM_WAYS),// A
    log2ceil(DCACHE_NUM_BANKS), // B
    XLEN,                   // address bits
    1,                      // number of ports
    DCACHE_NUM_REQS,        // number of inputs
    DCACHE_NUM_BANKS,       // number of outputs
    DCACHE_WRITEBACK,       // write-back
    false,                  // write response
    DCACHE_MSHR_SIZE,       // mshr size
    2,                      // pipeline latency
  });

  /*
  for (uint32_t i = 0; i < DCACHE_NUM_BANKS; ++i) {
    // TODO: add arbiter
    dcaches_->MemReqPorts.at(i).bind(&dcache_mem_req_ports.at(i));
    dcache_mem_rsp_ports.at(i).bind(&dcaches_->MemRspPorts.at(i));
  }
  */
  //std::cout << "HERE1" << std::endl;
  // create cores
  for (uint32_t i = 0; i < cores_per_socket; ++i) {
    uint32_t core_id = socket_id * cores_per_socket + i;
    cores_.at(i) = Core::Create(core_id, this, arch, dcrs);

    cores_.at(i)->icache_req_ports.at(0).bind(&icaches_->CoreReqPorts.at(i).at(0));
    icaches_->CoreRspPorts.at(i).at(0).bind(&cores_.at(i)->icache_rsp_ports.at(0));

    for (uint32_t j = 0; j < DCACHE_NUM_REQS; ++j) {
      cores_.at(i)->dcache_req_ports.at(j).bind(&dcaches_->CoreReqPorts.at(i).at(j));
      dcaches_->CoreRspPorts.at(i).at(j).bind(&cores_.at(i)->dcache_rsp_ports.at(j));
    }
  }
  //std::cout << "HERE2" << std::endl;
  auto dcache_switch = MemSwitch::Create(sname, ArbiterType::RoundRobin, DCACHE_NUM_BANKS);

  // USE FOR LATER
  auto l1_switch = MemSwitch::Create(sname, ArbiterType::RoundRobin, L1_NUM_MEM_PORTS, L1_NUM_MEM_PORTS);

  // arbiter for l1 cache as a whole
  for (uint32_t i = 0; i < DCACHE_NUM_BANKS; ++i) {
    dcaches_->MemReqPorts.at(i).bind(&dcache_switch->ReqIn.at(i));
    dcache_switch->RspIn.at(i).bind(&dcaches_->MemRspPorts.at(i));
  }
  //std::cout << "HERE3" << std::endl;
  // Temporary two bank l1 cache
  // icache
  icaches_->MemReqPorts.at(0).bind(&l1_mem_req_ports.at(0));
  l1_mem_rsp_ports.at(0).bind(&icaches_->MemRspPorts.at(0));

  //std::cout << "HERE4" << std::endl;
  // dcache
  dcache_switch->ReqOut.at(0).bind(&l1_mem_req_ports.at(1));
  l1_mem_rsp_ports.at(1).bind(&dcache_switch->RspOut.at(0));
  //std::cout << "HERE5" << std::endl;
}

Socket::~Socket() {
  //--
}

void Socket::reset() {
  //--
}

void Socket::tick() {
  //--
}

void Socket::attach_ram(RAM* ram) {
  for (auto core : cores_) {
    core->attach_ram(ram);
  }
}

#ifdef VM_ENABLE
void Socket::set_satp(uint64_t satp) {
  for (auto core : cores_) {
    core->set_satp(satp);
  }
}
#endif

bool Socket::running() const {
  for (auto& core : cores_) {
    if (core->running())
      return true;
  }
  return false;
}

int Socket::get_exitcode() const {
  int exitcode = 0;
  for (auto& core : cores_) {
    exitcode |= core->get_exitcode();
  }
  return exitcode;
}

void Socket::barrier(uint32_t bar_id, uint32_t count, uint32_t core_id) {
  cluster_->barrier(bar_id, count, socket_id_ * cores_.size() + core_id);
}

void Socket::resume(uint32_t core_index) {
  cores_.at(core_index)->resume(-1);
}

Socket::PerfStats Socket::perf_stats() const {
  PerfStats perf_stats;
  perf_stats.icache = icaches_->perf_stats();
  perf_stats.dcache = dcaches_->perf_stats();
  return perf_stats;
}