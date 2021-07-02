workspace "debugmenu"
	configurations { "Release" }
	location "build"

	files { "src/*.*" }

project "debugmenu"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"
	characterset ("MBCS")
	includedirs { os.getenv("RWSDK36") }


	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		flags { "StaticRuntime" }
