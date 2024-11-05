include "External/Dependencies/conandeps.premake5.lua" 

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "on" 

    
    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }
   
    includedirs
    {
        "src", 
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
        
        -- links
        -- { 
        -- 	"%{ExternalLibs.ShaderC_Debug}",
        -- 	"%{ExternalLibs.SPIRV_Cross_Debug}",
        -- 	"%{ExternalLibs.SPIRV_Cross_GLSL_Debug}",
        -- }
    
    filter "configurations:release"
        defines { "RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On" 
        
        -- links
        -- {
        -- 	"%{ExternalLibs.ShaderC_Release}",
        -- 	"%{ExternalLibs.SPIRV_Cross_Release}",
        -- 	"%{ExternalLibs.SPIRV_Cross_GLSL_Release}",
        -- } 

    -- include external dependencies
    conan_setup()