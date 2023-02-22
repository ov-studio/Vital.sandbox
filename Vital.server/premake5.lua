project "Vital.server"
    language "C++"
    cppdialect "C++17"
    kind "ConsoleApp"
    targetdir ("../.build/"..outputdir.."/Vital.multiplayer")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "../Vital.sandbox" }
    links { "Vital.sandbox" }
    files {
        "premake5.lua",
        "**.cpp",
        "**.c",
        "**.hpp",
        "**.h"
    }