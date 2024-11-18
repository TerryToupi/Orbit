#pragma once 

#include "src/core/assert.h"
#include "volk.h" 
 
#define VK_VALIDATE(check) ENGINE_ASSERT(check == VK_SUCCESS)
