project "ImGui"
	kind "StaticLib"
	language "C++" 
    cppdialect "C++20"

	ExternalIncludePaths["ImGui"] = "%{wks.location}/External/imgui"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

	files
	{
		"imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp"
	} 

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		staticruntime "On" 

    filter "system:macosx"
		pic "On"
		systemversion "latest"
		staticruntime "On"

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