project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    { 
        "include/**.h", 
        "src/**.cpp" 
    }

    includedirs 
    {
        "include",  
        "%{ExternalIncludePaths.VulkanSDK}", 
        "%{ExternalIncludePaths.Volk}", 
        "%{ExternalIncludePaths.SpdLog}", 
        "%{ExternalIncludePaths.GLFW}", 
        "%{ExternalIncludePaths.ImGui}"
    }   

    links
    {  
        "GLFW", 
        "ImGui",
        "Volk"
    }  

    defines
    {  
        -- remove for product
        "EDITOR_APPLICATION"
    }

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        defines 
        { 
            "OP_WINDOWS",
            "GLFW_INCLUDE_NONE",
            "VULKAN_BACKEND"
        } 

    filter "system:linux"
        defines 
        {
            "OP_LINUX",
            "GLFW_INCLUDE_NONE",
            "VULKAN_BACKEND"
        } 

    filter "system:macosx"
        defines 
        {
            "OP_MACOS",
            "GLFW_INCLUDE_NONE",
            "VULKAN_BACKEND"
        } 

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On" 

        links
		{ 
			"%{ExternalLibs.ShaderC_Debug}",
			"%{ExternalLibs.SPIRV_Cross_Debug}",
			"%{ExternalLibs.SPIRV_Cross_GLSL_Debug}",
		}

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On" 

        links
		{
			"%{ExternalLibs.ShaderC_Release}",
			"%{ExternalLibs.SPIRV_Cross_Release}",
			"%{ExternalLibs.SPIRV_Cross_GLSL_Release}",
		}

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"

        links
		{
			"%{ExternalLibs.ShaderC_Release}",
			"%{ExternalLibs.SPIRV_Cross_Release}",
			"%{ExternalLibs.SPIRV_Cross_GLSL_Release}",
		}