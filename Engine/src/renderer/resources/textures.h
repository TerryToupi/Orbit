#pragma once 

#include "src/utilities/handles.h"
#include "glm/glm.hpp"  
#include "stb_image.h"
#include "src/renderer/resources/enums.h" 

#define TEXTURE Handle<Texture>

namespace Engine
{
	class Texture;

	struct TextureDesc
	{ 
		const char* debugName; 
		glm::u32vec3 dimensions;
		uint32_t mipLevels = 1; 
		uint32_t arrayLayers = 1;
		TextureFormat format = TextureFormat::RGBA8_UNORM;
		TextureFormat internalFormat = TextureFormat::RGBA8_UNORM;
		TextureUsage usage = TextureUsage::TEXTURE_BINDING;
		TextureType type = TextureType::D2;
		TextureAspect aspect = TextureAspect::COLOR; 
		stbi_uc* data = nullptr;
	};
}
