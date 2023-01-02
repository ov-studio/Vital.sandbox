project "Vital.bundler"
    language "C++"
    cppdialect "C++17"
    kind "ConsoleApp"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "../Vital.sandbox" }
    links { "Vital.sandbox" }
    files {
        "premake5.lua",
        "**.c",
        "**.cpp",
        "**.h"
    }