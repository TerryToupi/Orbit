project "SpdLog"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "Binaries/%{cfg.buildcfg}"
    staticruntime "on"

    files 
    { 
        "src/**.h", 
        "src/**.cpp",
        "include/**.h",
        "include/**.cpp"
    }

    includedirs
    {
        "include", 
    } 

    ExteranlIncludePath["SpdLog"] = "%{wks.location}/External/spdlog-1.14.1/include"

    targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

    filter "system:windows"
        defines { "SPDLOG_COMPILED_LIB" } 

    filter "system:linux"
        defines { "SPDLOG_COMPILED_LIB" } 

    filter "system:macosx"
        defines { "SPDLOG_COMPILED_LIB" } 

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