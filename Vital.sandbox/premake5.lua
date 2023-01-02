project "Vital.sandbox"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "Vendor/openssl/include/openssl", "Vendor/fmod/include", "Vendor/lua", "Vendor/duktape" }
    files {
        "premake5.lua",
        "**.cpp",
        "**.h"
    }