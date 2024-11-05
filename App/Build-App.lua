project "Application"
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
        "%{wks.location}/Editor/src", 

        -- Engine lib include 
        "%{wks.location}/Engine/src", 
    } 

    links
    {
        "Engine",
        "Editor"
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

    filter "configurations:debug"
        defines { "DEBUG" }
        runtime "Debug"
        symbols "On" 

    filter "configurations:release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"  
