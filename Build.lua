-- premake5.lua
workspace "Orbit"
   architecture "x86_64"
   configurations { "Debug", "Release", "Dist" } 
   startproject "Editor"

   -- Global space flags
   flags
	{ 
		"MultiProcessorCompile"
	}

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { 
         "/EHsc", 
         "/Zc:preprocessor", 
         "/Zc:__cplusplus" 
      } 

-- External include paths 
ExteranlIncludePath = {}

-- predefined output directory
OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "External" 
    include "External/spdlog-1.14.1/Build.lua" 
group "" 

group "Engine"
    include "Engine/Build-Engine.lua"
group ""

group "Editor"
    include "Editor/Build-Editor.lua" 
group ""  

-- group "Tests"
--    include "Tests/Build-Tests.lua" 
-- group ""