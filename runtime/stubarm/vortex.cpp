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

#include <vortex.h>
#include <common.h>
#include <callbacks.h>

#include <unistd.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <dlfcn.h>
#include <iostream>

// callbacks.inc
struct vx_buffer {
  vx_device* device;
  uint64_t addr;
  uint64_t size;
};

extern int vx_dev_init(callbacks_t* callbacks) {
  if (nullptr == callbacks)
    return -1;

  callbacks->dev_open = [](vx_device_h* hdevice)->int {
    if (nullptr == hdevice)
      return  -1;
    auto device = new vx_device();
    if (device == nullptr)
      return -1;
    CHECK_ERR(device->init(), {
      delete device;
      return err;
    });
    DBGPRINT("DEV_OPEN: hdevice=%p\n", (void*)device);
    *hdevice = device;
    return 0;
  };

  callbacks->dev_close = [](vx_device_h hdevice)->int {
    if (nullptr == hdevice)
      return -1;
    DBGPRINT("DEV_CLOSE: hdevice=%p\n", hdevice);
    auto device = ((vx_device*)hdevice);
    delete device;
    return 0;
  };

  callbacks->dev_caps = [](vx_device_h hdevice, uint32_t caps_id, uint64_t *value)->int {
    if (nullptr == hdevice)
      return -1;
    vx_device *device = ((vx_device*)hdevice);
    uint64_t _value;
    CHECK_ERR(device->get_caps(caps_id, &_value), {
      return err;
    });
    DBGPRINT("DEV_CAPS: hdevice=%p, caps_id=%d, value=%ld\n", hdevice, caps_id, _value);
    *value = _value;
    return 0;
  };

  callbacks->mem_alloc = [](vx_device_h hdevice, uint64_t size, int flags, vx_buffer_h* hbuffer)->int {
    if (nullptr == hdevice
     || nullptr == hbuffer
     || 0 == size)
      return -1;
    auto device = ((vx_device*)hdevice);
    uint64_t dev_addr;
    CHECK_ERR(device->mem_alloc(size, flags, &dev_addr), {
      return err;
    });
    auto buffer = new vx_buffer{device, dev_addr, size};
    if (nullptr == buffer) {
      device->mem_free(dev_addr);
      return -1;
    }
    DBGPRINT("MEM_ALLOC: hdevice=%p, size=%ld, flags=0x%d, hbuffer=%p\n", hdevice, size, flags, (void*)buffer);
    *hbuffer = buffer;
    return 0;
  };

  callbacks->mem_reserve = [](vx_device_h hdevice, uint64_t address, uint64_t size, int flags, vx_buffer_h* hbuffer) {
    if (nullptr == hdevice
     || nullptr == hbuffer
     || 0 == size)
      return -1;
    auto device = ((vx_device*)hdevice);
    CHECK_ERR(device->mem_reserve(address, size, flags), {
      return err;
    });
    auto buffer = new vx_buffer{device, address, size};
    if (nullptr == buffer) {
      device->mem_free(address);
      return -1;
    }
    DBGPRINT("MEM_RESERVE: hdevice=%p, address=0x%lx, size=%ld, flags=0x%d, hbuffer=%p\n", hdevice, address, size, flags, (void*)buffer);
    *hbuffer = buffer;
    return 0;
  };

  callbacks->mem_free = [](vx_buffer_h hbuffer) {
    if (nullptr == hbuffer)
      return 0;
    DBGPRINT("MEM_FREE: hbuffer=%p\n", hbuffer);
    auto buffer = ((vx_buffer*)hbuffer);
    auto device = ((vx_device*)buffer->device);
    device->mem_access(buffer->addr, buffer->size, 0);
    int err = device->mem_free(buffer->addr);
    delete buffer;
    return err;
  };

  callbacks->mem_access = [](vx_buffer_h hbuffer, uint64_t offset, uint64_t size, int flags) {
    if (nullptr == hbuffer)
      return -1;
    auto buffer = ((vx_buffer*)hbuffer);
    auto device = ((vx_device*)buffer->device);
    if ((offset + size) > buffer->size)
      return -1;
    DBGPRINT("MEM_ACCESS: hbuffer=%p, offset=%ld, size=%ld, flags=%d\n", hbuffer, offset, size, flags);
    return device->mem_access(buffer->addr + offset, size, flags);
  };

  callbacks->mem_address = [](vx_buffer_h hbuffer, uint64_t* address) {
    if (nullptr == hbuffer)
      return -1;
    auto buffer = ((vx_buffer*)hbuffer);
    DBGPRINT("MEM_ADDRESS: hbuffer=%p, address=0x%lx\n", hbuffer, buffer->addr);
    *address = buffer->addr;
    return 0;
  };

  callbacks->mem_info = [](vx_device_h hdevice, uint64_t* mem_free, uint64_t* mem_used) {
    if (nullptr == hdevice)
      return -1;
    auto device = ((vx_device*)hdevice);
    uint64_t _mem_free, _mem_used;
    CHECK_ERR(device->mem_info(&_mem_free, &_mem_used), {
      return err;
    });
    DBGPRINT("MEM_INFO: hdevice=%p, mem_free=%ld, mem_used=%ld\n", hdevice, _mem_free, _mem_used);
    if (mem_free) {
      *mem_free = _mem_free;
    }
    if (mem_used) {
      *mem_used = _mem_used;
    }
    return 0;
  };

  callbacks->copy_to_dev = [](vx_buffer_h hbuffer, const void* host_ptr, uint64_t dst_offset, uint64_t size) {
    if (nullptr == hbuffer || nullptr == host_ptr)
      return -1;
    auto buffer = ((vx_buffer*)hbuffer);
    auto device = ((vx_device*)buffer->device);
    if ((dst_offset + size) > buffer->size)
      return -1;
    DBGPRINT("COPY_TO_DEV: hbuffer=%p, host_addr=%p, dst_offset=%ld, size=%ld\n", hbuffer, host_ptr, dst_offset, size);
    return device->upload(buffer->addr + dst_offset, host_ptr, size);
  };

  callbacks->copy_from_dev = [](void* host_ptr, vx_buffer_h hbuffer, uint64_t src_offset, uint64_t size) {
    if (nullptr == hbuffer || nullptr == host_ptr)
      return -1;
    auto buffer = ((vx_buffer*)hbuffer);
    auto device = ((vx_device*)buffer->device);
    if ((src_offset + size) > buffer->size)
      return -1;
    DBGPRINT("COPY_FROM_DEV: hbuffer=%p, host_addr=%p, src_offset=%ld, size=%ld\n", hbuffer, host_ptr, src_offset, size);
    return device->download(host_ptr, buffer->addr + src_offset, size);
  };

  callbacks->start = [](vx_device_h hdevice, vx_buffer_h hkernel, vx_buffer_h harguments) {
    if (nullptr == hdevice || nullptr == hkernel || nullptr == harguments)
      return -1;
    DBGPRINT("START: hdevice=%p, hkernel=%p, harguments=%p\n", hdevice, hkernel, harguments);
    auto device = ((vx_device*)hdevice);
    auto kernel = ((vx_buffer*)hkernel);
    auto arguments = ((vx_buffer*)harguments);
    return device->start(kernel->addr, arguments->addr);
  };

  callbacks->ready_wait = [](vx_device_h hdevice, uint64_t timeout) {
    if (nullptr == hdevice)
      return -1;
    DBGPRINT("READY_WAIT: hdevice=%p, timeout=%ld\n", hdevice, timeout);
    auto device = ((vx_device*)hdevice);
    return device->ready_wait(timeout);
  };

  callbacks->dcr_read = [](vx_device_h hdevice, uint32_t addr, uint32_t* value) {
    if (nullptr == hdevice || NULL == value)
      return -1;
    auto device = ((vx_device*)hdevice);
    uint32_t _value;
    CHECK_ERR(device->dcr_read(addr, &_value), {
      return err;
    });
    DBGPRINT("DCR_READ: hdevice=%p, addr=0x%x, value=0x%x\n", hdevice, addr, _value);
    *value = _value;
    return 0;
  };

  callbacks->dcr_write = [](vx_device_h hdevice, uint32_t addr, uint32_t value) {
    if (nullptr == hdevice)
      return -1;
    DBGPRINT("DCR_WRITE: hdevice=%p, addr=0x%x, value=0x%x\n", hdevice, addr, value);
    auto device = ((vx_device*)hdevice);
    return device->dcr_write(addr, value);
  };

  callbacks->mpm_query = [](vx_device_h hdevice, uint32_t addr, uint32_t core_id, uint64_t* value) {
    if (nullptr == hdevice)
      return -1;
    auto device = ((vx_device*)hdevice);
    uint64_t _value;
    CHECK_ERR(device->mpm_query(addr, core_id, &_value), {
      return err;
    });
    DBGPRINT("MPM_QUERY: hdevice=%p, addr=0x%x, core_id=%d, value=0x%lx\n", hdevice, addr, core_id, _value);
    *value = _value;
    return 0;
  };

  return 0;
}



int get_profiling_mode();

static int dcr_initialize(vx_device_h hdevice) {
  const uint64_t startup_addr(STARTUP_ADDR);

  CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_STARTUP_ADDR0, startup_addr & 0xffffffff), {
    return err;
  });

  CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_STARTUP_ADDR1, startup_addr >> 32), {
    return err;
  });

  CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_STARTUP_ARG0, 0), {
    return err;
  });

  CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_STARTUP_ARG1, 0), {
    return err;
  });

  CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_MPM_CLASS, 0), {
    return err;
  });

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

static callbacks_t g_callbacks;

typedef int (*vx_dev_init_t)(callbacks_t*);

extern int vx_dev_open(vx_device_h* hdevice) {
  {
    vx_dev_init(&g_callbacks);
  }

  vx_device_h _hdevice;

  CHECK_ERR((g_callbacks.dev_open)(&_hdevice), {
    return err;
  });

  CHECK_ERR(dcr_initialize(_hdevice), {
    return err;
  });

  *hdevice = _hdevice;

  return 0;
}

extern int vx_dev_close(vx_device_h hdevice) {
  vx_dump_perf(hdevice, stdout);
  int ret = (g_callbacks.dev_close)(hdevice);
  return ret;
}

extern int vx_dev_caps(vx_device_h hdevice, uint32_t caps_id, uint64_t* value) {
  return (g_callbacks.dev_caps)(hdevice, caps_id, value);
}

extern int vx_mem_alloc(vx_device_h hdevice, uint64_t size, int flags, vx_buffer_h* hbuffer) {
  return (g_callbacks.mem_alloc)(hdevice, size, flags, hbuffer);
}

extern int vx_mem_reserve(vx_device_h hdevice, uint64_t address, uint64_t size, int flags, vx_buffer_h* hbuffer) {
  return (g_callbacks.mem_reserve)(hdevice, address, size, flags, hbuffer);
}

extern int vx_mem_free(vx_buffer_h hbuffer) {
  return (g_callbacks.mem_free)(hbuffer);
}

extern int vx_mem_access(vx_buffer_h hbuffer, uint64_t offset, uint64_t size, int flags) {
  return (g_callbacks.mem_access)(hbuffer, offset, size, flags);
}

extern int vx_mem_address(vx_buffer_h hbuffer, uint64_t* address) {
  return (g_callbacks.mem_address)(hbuffer, address);
}

extern int vx_mem_info(vx_device_h hdevice, uint64_t* mem_free, uint64_t* mem_used) {
  return (g_callbacks.mem_info)(hdevice, mem_free, mem_used);
}

extern int vx_copy_to_dev(vx_buffer_h hbuffer, const void* host_ptr, uint64_t dst_offset, uint64_t size) {
  return (g_callbacks.copy_to_dev)(hbuffer, host_ptr, dst_offset, size);
}

extern int vx_copy_from_dev(void* host_ptr, vx_buffer_h hbuffer, uint64_t src_offset, uint64_t size) {
  return (g_callbacks.copy_from_dev)(host_ptr, hbuffer, src_offset, size);
}

extern int vx_start(vx_device_h hdevice, vx_buffer_h hkernel, vx_buffer_h harguments) {
  int profiling_mode = get_profiling_mode();
  if (profiling_mode != 0) {
    CHECK_ERR(vx_dcr_write(hdevice, VX_DCR_BASE_MPM_CLASS, profiling_mode), {
      return err;
    });
  }
  return (g_callbacks.start)(hdevice, hkernel, harguments);
}

extern int vx_ready_wait(vx_device_h hdevice, uint64_t timeout) {
  return (g_callbacks.ready_wait)(hdevice, timeout);
}

extern int vx_dcr_read(vx_device_h hdevice, uint32_t addr, uint32_t* value) {
  return (g_callbacks.dcr_read)(hdevice, addr, value);
}

extern int vx_dcr_write(vx_device_h hdevice, uint32_t addr, uint32_t value) {
  return (g_callbacks.dcr_write)(hdevice, addr, value);
}

extern int vx_mpm_query(vx_device_h hdevice, uint32_t addr, uint32_t core_id, uint64_t* value) {
  if (core_id == 0xffffffff) {
    uint64_t num_cores;
    CHECK_ERR((g_callbacks.dev_caps)(hdevice, VX_CAPS_NUM_CORES, &num_cores), {
      return err;
    });
    uint64_t sum_value = 0;
    uint64_t cur_value;
    for (uint32_t i = 0; i < num_cores; ++i) {
      CHECK_ERR((g_callbacks.mpm_query)(hdevice, addr, i, &cur_value), {
        return err;
      });
      sum_value += cur_value;
    }
    *value = sum_value;
    return 0;
  } else {
    return (g_callbacks.mpm_query)(hdevice, addr, core_id, value);
  }
}