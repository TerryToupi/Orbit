#include "platform/Vulkan/resources/GfxVkEnums.h"

namespace Engine
{ 
	VkImageUsageFlags VkEnums::TextureUsageToVkVkImageUsageFlags(TextureUsage usage)
	{
		switch (usage)
		{
		case TextureUsage::COPY_SRC:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case TextureUsage::COPY_DST:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case TextureUsage::TEXTURE_BINDING:
			return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		case TextureUsage::STORAGE_BINDING:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case TextureUsage::RENDER_ATTACHMENT:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case TextureUsage::SAMPLED:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case TextureUsage::DEPTH_STENCIL:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		default:
			break;
		}

		return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
	} 

	VkImageType VkEnums::TextureTypeToVkImageType(TextureType type)
	{
		switch (type)
		{
		case TextureType::D1:
			return VK_IMAGE_TYPE_1D;
		case TextureType::D2:
			return VK_IMAGE_TYPE_2D;
		case TextureType::D3:
			return VK_IMAGE_TYPE_3D;
		default:
			break;
		}

		return VK_IMAGE_TYPE_MAX_ENUM;
	} 

	VkFormat VkEnums::TextureFormatToVkFormat(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::RGB32_FLOAT:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case TextureFormat::D32_FLOAT:
			return VK_FORMAT_D32_SFLOAT;
		case TextureFormat::RGBA16_FLOAT:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case TextureFormat::RGBA8_UNORM:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RG16_FLOAT:
			return VK_FORMAT_R16G16_SFLOAT;
		case TextureFormat::R32_FLOAT:
			return VK_FORMAT_R32_SFLOAT;
		case TextureFormat::RGBA8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::BGRA8_UNORM:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::BGRA8_SRGB:
			return VK_FORMAT_B8G8R8A8_SRGB;
		}

		return VK_FORMAT_MAX_ENUM;
	} 

	VkImageAspectFlags VkEnums::TextureAspectToVkImageAspectFlags(TextureAspect aspect)
	{
		switch (aspect)
		{
		case TextureAspect::NONE:
			return VK_IMAGE_ASPECT_NONE;
		case TextureAspect::COLOR:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		case TextureAspect::DEPTH:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case TextureAspect::STENCIL:
			return VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
	} 

	VkImageViewType VkEnums::TextureTypeToVkVkImageViewType(TextureType textureType)
	{
		switch (textureType)
		{
		case TextureType::D1:
			return VK_IMAGE_VIEW_TYPE_1D;
		case TextureType::D2:
			return VK_IMAGE_VIEW_TYPE_2D;
		case TextureType::D3:
			return VK_IMAGE_VIEW_TYPE_3D;
		default:
			break;
		}

		return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
	} 

	VkFilter VkEnums::FilterToVkFilter(Filter filter)
	{
		switch (filter)
		{
		case Filter::NEAREST:
			return VK_FILTER_NEAREST;
		case Filter::LINEAR:
			return VK_FILTER_LINEAR;
		case Filter::CUBIC:
			return VK_FILTER_CUBIC_EXT;
		}

		return VK_FILTER_MAX_ENUM;
	} 

	VkSamplerMipmapMode VkEnums::FilterToVkVkSamplerMipmapMode(Filter filter)
	{
		switch (filter)
		{
		case Filter::NEAREST:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case Filter::LINEAR:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		} 

		return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
	}

	VkSamplerAddressMode VkEnums::WrapToVkSamplerAddressMode(Wrap wrap)
	{
		switch (wrap)
		{
		case Wrap::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case Wrap::REPEAT_MIRRORED:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case Wrap::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case Wrap::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case Wrap::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		}

		return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
	} 

	VkCompareOp VkEnums::CompareToVkCompareOp(Compare compare)
	{
		switch (compare)
		{
		case Compare::ALAWAYS:
			return VK_COMPARE_OP_ALWAYS;
		case Compare::NEVER:
			return VK_COMPARE_OP_NEVER;
		case Compare::LESS:
			return VK_COMPARE_OP_LESS;
		case Compare::LESS_OR_EQUAL:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case Compare::GREATER:
			return VK_COMPARE_OP_GREATER;
		case Compare::GREATER_OR_EQUAL:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case Compare::EQUAL:
			return VK_COMPARE_OP_EQUAL;
		case Compare::NOT_EQUAL:
			return VK_COMPARE_OP_NOT_EQUAL;
		}

		return VK_COMPARE_OP_MAX_ENUM;
	}
}
