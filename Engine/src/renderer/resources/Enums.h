#pragma once 

namespace Engine
{
	enum class ShaderType
	{
		VERTEX = 0x1,
		FRAGMENT = 0x2,
		COMPUTE = 0x3,
	}; 

	enum class BufferBindingType
	{
		UNIFORM = 0x0, 
		UNIFORM_DYNAMIC_OFFSET = 0x1,
		STORAGE = 0x2,
		READ_ONLY_STORAGE = 0x3,
		READ_WRITE_STORAGE = 0x4, 
	}; 

	enum class Compare
	{ 
		NEVER = 0x0,
		LESS = 0x1,
		LESS_OR_EQUAL = 0x2,
		GREATER = 0x3,
		GREATER_OR_EQUAL = 0x4,
		EQUAL = 0x5,
		NOT_EQUAL = 0x6,
		ALAWAYS = 0x7,
		NONE = 0x8,
	}; 

	enum class VertexFormat
	{
		F32	  = 0x0,
		F32x2 = 0x1,
		F32x3 = 0x2,
		F32x4 = 0x3,
		I32	  = 0x4,
		I32x2 = 0x5,
		I32x3 = 0x6,
		I32x4 = 0x7,
		U32	  = 0x8,
		U32x2 = 0x9,
		U32x3 = 0x10,
		U32x4 = 0x11,
	}; 

	enum class Filter
	{
		NEAREST = 0x0,
		LINEAR = 0x1,
		CUBIC = 0x2,
	}; 

	enum class Wrap
	{
		REPEAT = 0x0,
		REPEAT_MIRRORED = 0x1,
		CLAMP_TO_EDGE = 0x2,
		CLAMP_TO_BORDER = 0x3,
		MIRROR_CLAMP_TO_EDGE = 0x4,
	};

	enum class Topology
	{
		POINT_LIST = 0x0,
		LINE_LIST = 0x1,
		LINE_STRIP = 0x2,
		TRIANGLE_LIST = 0x3,
		TRIANGLE_STRIP = 0x4,
		TRIANGLE_FAN = 0x5,
		PATCH_LIST = 0x6,
	};

	enum class PolygonMode
	{
		FILL = 0x0,
		LINE = 0x1,
		POINT = 0x2,
	};

	enum class CullMode
	{
		NONE = 0x0,
		FRONT_FACE = 0x1,
		BACK_FACE = 0x2,
		FRONT_AND_BACK = 0x3,
	};

	enum class TextureFormat
	{
		RGB32_FLOAT = 0x1,
		D32_FLOAT = 0x2,
		RGBA16_FLOAT = 0x3,
		RGBA8_UNORM = 0x4,
		RG16_FLOAT = 0x5,
		RGBA8_FLOAT = 0x6,
		R32_FLOAT = 0x7,
		RGBA8_SRGB = 0x8,
		RGBA8_LINEAR = 0x9,
		BGRA8_UNORM = 0x10,
		BGRA8_SRGB = 0x11,
	};

	enum class MemoryUsage
	{
		CPU_ONLY = 0x0,
		GPU_ONLY = 0x1,
		GPU_CPU = 0x2,
		CPU_GPU = 0x3,
	};

	enum class BufferUsage
	{
		MAP_READ = 0x1,
		MAP_WRITE = 0x2,
		COPY_SRC = 0x3,
		COPY_DST = 0x4,
		INDEX = 0x5,
		VERTEX = 0x6,
		UNIFORM = 0x7,
		STORAGE = 0x8,
		INDIRECT = 0x9,
		QUERY_RESOLVE = 0x10,
	};

	enum class BufferUsageHint
	{
		STATIC = 0x1,
		DYNAMIC = 0x2,
	};

	enum class TextureType
	{
		D1 = 0x0,
		D2 = 0x1,
		D3 = 0x2,
	};

	enum class TextureAspect
	{
		NONE = 0x0,
		COLOR = 0x1,
		DEPTH = 0x2,
		STENCIL = 0x3,
	};

	enum class TextureUsage
	{
		COPY_SRC = 0x1,
		COPY_DST = 0x2,
		TEXTURE_BINDING = 0x3,
		STORAGE_BINDING = 0x4,
		RENDER_ATTACHMENT = 0x5,
		SAMPLED = 0x6,
		DEPTH_STENCIL = 0x7,
	};

	enum class TextureLayout
	{
		UNDEFINED = 0x0,
		COPY_SRC = 0x1,
		COPY_DST = 0x2,
		RENDER_ATTACHMENT = 0x3,
		DEPTH_STENCIL = 0x4,
		PRESENT = 0x5,
	};

	enum class BlendOperation
	{
		ADD = 0x0,
		MUL = 0x1,
		SUB = 0x2,
		MIN = 0x3,
		MAX = 0x4,
	};

	enum class BlendFactor
	{
		SRC_ALPHA = 0x0,
		ONE_MINUS_SRC_ALPHA = 0x1,
		ONE = 0x2,
		ZERO = 0x3,
	};

	enum class StoreOperation
	{
		STORE = 0x1,
		DONT_CARE = 0x2,
	};

	enum class LoadOperation
	{
		CLEAR = 0x0,
		LOAD = 0x1,
		DONT_CARE = 0x2,
	};
}
