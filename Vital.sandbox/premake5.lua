project "Vital.sandbox"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "Vendor", "Vendor/openssl/include", "Vendor/fmod/include", "Vendor/lua", "Vendor/duktape" }
    filter "configurations:Debug"
        links { "Vendor/openssl/x64/Debug/libcrypto.lib", "Vendor/openssl/x64/Debug/libssl.lib", "Vendor/fmod/lib/x64/fmodL_vc.lib" }
    filter "configurations:Release"
        links { "Vendor/openssl/x64/Release/libcrypto.lib", "Vendor/openssl/x64/Release/libssl.lib", "Vendor/fmod/lib/x64/fmod_vc.lib" }
    postbuildcommands {
        "{COPY} %{cfg.objdir}/output.map %{cfg.targetdir}"
    }
    files {
        "premake5.lua",
        "**.cpp",
        "**.h"
    }