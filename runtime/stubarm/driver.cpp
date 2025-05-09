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

#include "driver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <dlfcn.h>
#include <string>
#include <vector>
#include <sstream>

int drv_init(opae_drv_api_t* opae_drv_funcs) {
  if (opae_drv_funcs == nullptr)
    return -1;

  opae_drv_funcs->fpgaGetProperties = *fpgaGetProperties;
  opae_drv_funcs->fpgaPropertiesSetObjectType = fpgaPropertiesSetObjectType;
  opae_drv_funcs->fpgaPropertiesSetGUID = fpgaPropertiesSetGUID;
  opae_drv_funcs->fpgaDestroyProperties = fpgaDestroyProperties;
  opae_drv_funcs->fpgaDestroyToken = fpgaDestroyToken;
  opae_drv_funcs->fpgaPropertiesGetLocalMemorySize = fpgaPropertiesGetLocalMemorySize;
  opae_drv_funcs->fpgaEnumerate = fpgaEnumerate;
  opae_drv_funcs->fpgaOpen = fpgaOpen;
  opae_drv_funcs->fpgaClose = fpgaClose;
  opae_drv_funcs->fpgaPrepareBuffer = fpgaPrepareBuffer;
  opae_drv_funcs->fpgaReleaseBuffer = fpgaReleaseBuffer;
  opae_drv_funcs->fpgaGetIOAddress = fpgaGetIOAddress;
  opae_drv_funcs->fpgaWriteMMIO64 = fpgaWriteMMIO64;
  opae_drv_funcs->fpgaReadMMIO64 = fpgaReadMMIO64;
  opae_drv_funcs->fpgaErrStr = fpgaErrStr;

  return 0;
}

