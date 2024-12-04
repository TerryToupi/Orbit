#pragma once 

#include "src/renderer/resources/meshes.h" 

namespace Engine
{
	class GfxVkMesh
	{ 
	public: 
		GfxVkMesh() = default; 
		GfxVkMesh(const MeshDesc&& desc) {} 

		void destroy() {}
	private:
	};
}