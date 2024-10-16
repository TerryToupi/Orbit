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


OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Engine"
	include "Engine/Build-Engine.lua"
group ""

group "Editor"
   include "Editor/Build-Editor.lua" 
group "" 

-- group "Tests"
--    include "Tests/Build-Tests.lua" 
-- group ""