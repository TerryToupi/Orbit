#pragma once 

#include "src/core/assert.h"
#include "volk.h" 
 
#define VK_VALIDATE(check) if(!(check == VK_SUCCESS)) { ENGINE_ASSERT(false, "[VULKAN ASSERT] " ENGINE_STRINGIFY_MACRO(check)); }
