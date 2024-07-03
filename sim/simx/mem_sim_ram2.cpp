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

#include "mem_sim_ram2.h"
#include <vector>
#include <queue>
#include <stdlib.h>
//#include <yaml-cpp/yaml.h>

#pragma push_macro("warn")
#undef warn

DISABLE_WARNING_PUSH
DISABLE_WARNING_UNUSED_PARAMETER
#define RAMULATOR
#include <ramulator2/src/base/base.h>
#include <ramulator2/src/base/request.h>
#include <ramulator2/src/base/config.h>
#include <ramulator2/src/frontend/frontend.h>
#include <ramulator2/src/memory_system/memory_system.h>
DISABLE_WARNING_POP

#include "constants.h"
#include "types.h"
#include "debug.h"

using namespace vortex;

class MemSim2::Impl {
private:
	MemSim2* simobject_;
	Config config_;
	PerfStats perf_stats_;
	std::string config_path_;
	Ramulator::IFrontEnd* ramulator2_frontend_;
	Ramulator::IMemorySystem* ramulator2_memorysystem_;
public:
	Impl(MemSim2 *simobject, const Config &config)
		: simobject_(simobject)
		, config_(config)
	{
		config_path_ = "../../../sim/mem_config.yaml";
		YAML::Node yaml_config = Ramulator::Config::parse_config_file(config_path_, {});

		ramulator2_frontend_ = Ramulator::Factory::create_frontend(yaml_config);
		ramulator2_memorysystem_ = Ramulator::Factory::create_memory_system(yaml_config);

		ramulator2_frontend_->connect_memory_system(ramulator2_memorysystem_);
		ramulator2_memorysystem_->connect_frontend(ramulator2_frontend_);
	}

	~Impl() {
		ramulator2_frontend_->finalize();
        ramulator2_memorysystem_->finalize();
		delete ramulator2_frontend_;
		delete ramulator2_memorysystem_;
	}

	const PerfStats& perf_stats() const {
		return perf_stats_;
	}

	void dram_callback(Ramulator::Request& req, uint32_t id, uint32_t tag, uint64_t uuid) {
		if (req.type_id == Ramulator::Request::Type::Write)
			return;
		MemRsp mem_rsp{tag, (uint32_t)req.source_id, uuid};
		simobject_->MemRspPorts.at(id).push(mem_rsp, 1);
		DT(3, simobject_->name() << "-" << mem_rsp);
	}

	uint32_t tick_helper(uint32_t i) {
		if (simobject_->MemReqPorts.at(i).empty())
			return 0;
		
		auto& mem_req = simobject_->MemReqPorts.at(i).front();

		/*
		Ramulator::Request dram_req( 
			mem_req.addr,
			mem_req.write ? Ramulator::Request::Type::Write : Ramulator::Request::Type::Read,
			mem_req.cid,
			std::bind(&Impl::dram_callback, this, placeholders::_1, i, mem_req.tag, mem_req.uuid)
		);
		*/

		if (!ramulator2_frontend_->
				receive_external_requests(mem_req.write, mem_req.addr, 0, 
				[this, i, mem_req](Ramulator::Request& req) {
					dram_callback(req, i, mem_req.tag, mem_req.uuid);
				}))
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
				ramulator2_memorysystem_->tick();
		} else {
			for (int i = MEM_CYCLE_RATIO; i <= 0; ++i)
				ramulator2_memorysystem_->tick();
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

MemSim2::MemSim2(const SimContext& ctx, const char* name, const Config& config) 
	: SimObject<MemSim2>(ctx, name)
	, MemReqPorts(L3_NUM_BANKS, this) 
	, MemRspPorts(L3_NUM_BANKS, this)
	, impl_(new Impl(this, config))
{}

MemSim2::~MemSim2() {
  delete impl_;
}

void MemSim2::reset() {
  impl_->reset();
}

void MemSim2::tick() {
  impl_->tick();
}

const MemSim2::PerfStats &MemSim2::perf_stats() const {
	return impl_->perf_stats();
}