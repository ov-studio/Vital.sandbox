workspace "module"
	configurations { "Debug", "Release" }
	location "."
	platforms { "x86", "x64" }
    outputdir = "%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}"
    include "src"

    filter "system:windows"
        defines { "WINDOWS", "WIN32" }

    filter "configurations:Debug"
        defines { "DEBUG" }

    filter "configurations:Release"
        optimize "On"