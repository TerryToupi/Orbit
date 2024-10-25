project "Volk"
	kind "StaticLib"
	language "C"

	files
	{ 
        "volk.h",
        "volk.c"
	}  

	includedirs 
	{
        "%{ExternalIncludePaths.VulkanSDK}"
	}

	ExternalIncludePaths["Volk"] = "%{wks.location}/External/volk-1.3.295"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	filter "system:linux"
		staticruntime "On"
        -- defines { "VK_USE_PLATFORM_XLIB_KHR" }

	filter "system:windows"
		staticruntime "On" 
        -- defines { "VK_USE_PLATFORM_WIN32_KHR" }

	filter "system:macosx"
		staticruntime "On"  
        -- defines { "VK_USE_PLATFORM_MACOS_MVK" }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		optimize "On" 

    filter "configurations:Dist"
        runtime "Release"
        optimize "On"
        symbols "Off"