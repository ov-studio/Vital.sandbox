project "Vital.sandbox"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    filter "configurations:Debug"
        includedirs { ".", "Vendor", "Vendor/openssl/x64/Debug/include/openssl", "Vendor/fmod/include", "Vendor/lua", "Vendor/duktape" }
    filter "configurations:Release"
        includedirs { ".", "Vendor", "Vendor/openssl/x64/Release/include/openssl", "Vendor/fmod/include", "Vendor/lua", "Vendor/duktape" }
    files {
        "premake5.lua",
        "**.cpp",
        "**.h"
    }