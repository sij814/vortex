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
#include <string>
#include <vector>
#include <sstream>

int drv_init(opae_drv_api_t* api)
{
    if (!api) return -1;

    api->fpgaGetProperties               = ::fpgaGetProperties;
    api->fpgaPropertiesSetObjectType     = ::fpgaPropertiesSetObjectType;
    api->fpgaPropertiesSetGUID           = ::fpgaPropertiesSetGUID;
    api->fpgaDestroyProperties           = ::fpgaDestroyProperties;
    api->fpgaEnumerate                   = ::fpgaEnumerate;
    api->fpgaDestroyToken                = ::fpgaDestroyToken;
    api->fpgaPropertiesGetLocalMemorySize= ::fpgaPropertiesGetLocalMemorySize;
    api->fpgaOpen                        = ::fpgaOpen;
    api->fpgaClose                       = ::fpgaClose;
    api->fpgaPrepareBuffer               = ::fpgaPrepareBuffer;
    api->fpgaReleaseBuffer               = ::fpgaReleaseBuffer;
    api->fpgaGetIOAddress                = ::fpgaGetIOAddress;
    api->fpgaWriteMMIO64                 = ::fpgaWriteMMIO64;
    api->fpgaReadMMIO64                  = ::fpgaReadMMIO64;
    api->fpgaErrStr                      = ::fpgaErrStr;

    return 0;
}

void drv_close()
{
    // Nothing to clean up in a statically linked setup
}