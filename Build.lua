-- premake5.lua
workspace "Orbit"
   configurations { "Debug", "Release" } 
   startproject "Application"

   -- Global space flags
   flags
	{ 
		"MultiProcessorCompile"
	}

   -- Workspace-wide build options for MSVC
   filter "system:windows" 
      architecture "amd64"
      buildoptions { 
         "/EHsc", 
         "/Zc:preprocessor", 
         "/Zc:__cplusplus" 
      }  

   filter "system:macosx"
      architecture "universal"
      xcodebuildsettings {
         ["ALWAYS_SEARCH_USER_PATHS"] = "NO",
         ["CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER"] = "YES"
      }

-- External include paths 
ExternalIncludePaths = {} 
ExternalLibPaths = {} 
ExternalLibs = {}

-- Vulkan setup
VULKAN_SDK = os.getenv("VULKAN_SDK") 

-- main Vulkan include dir
ExternalIncludePaths["VulkanSDK"] = "%{VULKAN_SDK}/Include" 

-- Vulkan lib folder
ExternalLibPaths["VulkanSDK"] = "%{VULKAN_SDK}/Lib" 

-- Vulkan in use libs
ExternalLibs["Vulkan"] = "%{ExternalLibPaths.VulkanSDK}/vulkan-1.lib"
ExternalLibs["VulkanUtils"] = "%{ExternalLibPaths.VulkanSDK}/VkLayer_utils.lib"
ExternalLibs["ShaderC_Debug"] = "%{ExternalLibPaths.VulkanSDK}/shaderc_sharedd.lib"
ExternalLibs["SPIRV_Cross_Debug"] = "%{ExternalLibPaths.VulkanSDK}/spirv-cross-cored.lib"
ExternalLibs["SPIRV_Cross_GLSL_Debug"] = "%{ExternalLibPaths.VulkanSDK}/spirv-cross-glsld.lib"
ExternalLibs["SPIRV_Tools_Debug"] = "%{ExternalLibPaths.VulkanSDK}/SPIRV-Toolsd.lib"
ExternalLibs["ShaderC_Release"] = "%{ExternalLibPaths.VulkanSDK}/shaderc_shared.lib"
ExternalLibs["SPIRV_Cross_Release"] = "%{ExternalLibPaths.VulkanSDK}/spirv-cross-core.lib"
ExternalLibs["SPIRV_Cross_GLSL_Release"] = "%{ExternalLibPaths.VulkanSDK}/spirv-cross-glsl.lib"

-- predefined output directory
OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "External"  
    include "External/volk-1.3.295/Build.lua"
    include "External/spdlog-1.14.1/Build.lua"   
    include "External/imgui/Build.lua"
    include "External/glfw-3.4/Build.lua"
group "" 

group "Engine"
    include "Engine/Build-Engine.lua"
group ""

group "Editor"
    include "Editor/Build-Editor.lua" 
group ""   

group "Application"  
    include "App/Build-App.lua"
group ""

-- group "Tests"
--    include "Tests/Build-Tests.lua" 
-- group ""