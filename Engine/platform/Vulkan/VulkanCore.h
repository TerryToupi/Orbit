#pragma once 

#include "src/core/assert.h" 

#define VK_ENABLE_BETA_EXTENSIONS
#include "volk.h"   

#include "vma/vk_mem_alloc.h"
 
#define VK_VALIDATE(check) if(!(check == VK_SUCCESS)) { ENGINE_ASSERT(false, "[VULKAN ASSERT] " ENGINE_STRINGIFY_MACRO(check)); }
