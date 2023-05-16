workspace "Vital.sandbox"
    configurations { "Debug", "Release" }
    location ".solution"
    platforms { "x86", "x64" }
    buildoptions { "/utf-8" }
    outputdir = "%{cfg.system}-%{cfg.platform}-%{cfg.buildcfg}"

    filter "system:windows"
        defines { "WINDOWS", "WIN32" }
    filter "configurations:Debug"
        defines { "DEBUG" }
    filter "configurations:Release"
        optimize "On"

    include "Vital.bundler"
    include "Vital.sandbox"
    include "Vital.client"
    include "Vital.server"