project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }

    externalincludedirs
    {
        "src", 
        
        -- External lib include
         "%{ExternalIncludePaths.SpdLog}",  

        -- Engine lib include 
        "%{wks.location}/Engine/src", 
    }

    links
    {  
        "Engine",
    }

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        defines 
        { 
            "OP_WINDOWS",
            "VULKAN_BACKEND"
        } 

    filter "system:linux"
        defines 
        {
            "OP_LINUX",
            "VULKAN_BACKEND"
        } 

    filter "system:macosx"
        defines 
        {
            "OP_MACOS",
            "VULKAN_BACKEND"
        }  

    filter "configurations:Debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        defines { "DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"