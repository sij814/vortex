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

#include "mem_sim.h"
#include <vector>
#include <queue>
#include <stdlib.h>

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNUSED_PARAMETER
#define RAMULATOR
#include <ramulator/src/Gem5Wrapper.h>
#include <ramulator/src/Request.h>
#include <ramulator/src/Statistics.h>
DISABLE_WARNING_POP

#include "constants.h"
#include "types.h"
#include "debug.h"

using namespace vortex;

class MemSim::Impl {
private:
	MemSim* simobject_;
	Config config_;
	PerfStats perf_stats_;
	ramulator::Gem5Wrapper* dram_;

public:
	Impl(MemSim *simobject, const Config &config)
		: simobject_(simobject)
		, config_(config)
	{
		ramulator::Config ram_config;
		ram_config.add("standard", "HBM");
		ram_config.add("channels", std::to_string(config.channels));
		ram_config.add("ranks", "1");
		ram_config.add("speed", "HBM_1Gbps");
		ram_config.add("org", "HBM_4Gb");
		ram_config.add("mapping", "defaultmapping");
		ram_config.set_core_num(config.num_cores);
		dram_ = new ramulator::Gem5Wrapper(ram_config, MEM_BLOCK_SIZE);
		Stats::statlist.output("ramulator.hbm.log");
	}
	/*
	Impl(MemSim* simobject, const Config& config) 
		: simobject_(simobject)
		, config_(config)
	{
		ramulator::Config ram_config;
		ram_config.add("standard", "DDR4");
		ram_config.add("channels", std::to_string(config.channels));
		ram_config.add("ranks", "1");
		ram_config.add("speed", "DDR4_2400R");
		ram_config.add("org", "DDR4_4Gb_x8");
		ram_config.add("mapping", "defaultmapping");
		ram_config.set_core_num(config.num_cores);
		dram_ = new ramulator::Gem5Wrapper(ram_config, MEM_BLOCK_SIZE);
		Stats::statlist.output("ramulator.ddr4.log");
	}
	*/

	~Impl() {
		dram_->finish();
		Stats::statlist.printall();
		delete dram_;
	}

	const PerfStats& perf_stats() const {
		return perf_stats_;
	}

	void dram_callback(ramulator::Request& req, uint32_t id, uint32_t tag, uint64_t uuid) {
		if (req.type == ramulator::Request::Type::WRITE)
			return;
		MemRsp mem_rsp{tag, (uint32_t)req.coreid, uuid};
		simobject_->MemRspPorts.at(id).push(mem_rsp, 1);
		DT(3, simobject_->name() << "-" << mem_rsp);
	}

	uint32_t tick_helper(uint32_t i) {
		if (simobject_->MemReqPorts.at(i).empty())
			return 0;
		
		auto& mem_req = simobject_->MemReqPorts.at(i).front();

		ramulator::Request dram_req( 
			mem_req.addr,
			mem_req.write ? ramulator::Request::Type::WRITE : ramulator::Request::Type::READ,
			std::bind(&Impl::dram_callback, this, placeholders::_1, i, mem_req.tag, mem_req.uuid),
			mem_req.cid
		);

		if (!dram_->send(dram_req))
			return 0;

		DT(3, simobject_->name() << "-" << mem_req);

		simobject_->MemReqPorts.at(i).pop();
		return 1;
	}

	void reset() {
		perf_stats_ = PerfStats();
	}

	void tick() {
		uint32_t counter = 0;
		if (MEM_CYCLE_RATIO > 0) {
			auto cycle = SimPlatform::instance().cycles();
			if ((cycle % MEM_CYCLE_RATIO) == 0)
				dram_->tick();
		} else {
			for (int i = MEM_CYCLE_RATIO; i <= 0; ++i)
				dram_->tick();
		}

		for (uint32_t i = 0; i < L3_NUM_BANKS; ++i)
		{
			counter += tick_helper(i);
		}
		perf_stats_.counter += counter;
		if (counter > 0) {
			++perf_stats_.ticks;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

MemSim::MemSim(const SimContext& ctx, const char* name, const Config& config) 
	: SimObject<MemSim>(ctx, name)
	, MemReqPorts(L3_NUM_BANKS, this) 
	, MemRspPorts(L3_NUM_BANKS, this)
	, impl_(new Impl(this, config))
{}

MemSim::~MemSim() {
  delete impl_;
}

void MemSim::reset() {
  impl_->reset();
}

void MemSim::tick() {
  impl_->tick();
}

const MemSim::PerfStats &MemSim::perf_stats() const {
	return impl_->perf_stats();
}