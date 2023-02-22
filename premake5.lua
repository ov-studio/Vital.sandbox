workspace "Vital.sandbox"
    configurations { "Debug", "Release" }
    location ".solution"
    platforms { "x86", "x64" }
    outputdir = "%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}"
    include "Vital.bundler"
    include "Vital.sandbox"
    include "Vital.client"

    filter "system:windows"
        defines { "WINDOWS", "WIN32" }
    filter "configurations:Debug"
        defines { "DEBUG" }
    filter "configurations:Release"
        optimize "On"