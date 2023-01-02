project "Vital.sandbox"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "Vendor", "Vendor/openssl/include", "Vendor/fmod/include", "Vendor/lua", "Vendor/duktape" }
    filter "configurations:Debug"
        links { "Vendor/openssl/lib/x64/Debug/libcrypto.lib", "Vendor/openssl/lib/x64/Debug/libssl.lib", "Vendor/fmod/lib/x64/fmodL_vc.lib" }
        postbuildcommands {
            "{COPY} Vendor/openssl/lib/x64/Debug/ossl_static.pdb %{cfg.targetdir}",
            "{COPY} Vendor/fmod/lib/x64/fmodL_vc.lib %{cfg.targetdir}"
        }
    filter "configurations:Release"
        links { "Vendor/openssl/lib/x64/Release/libcrypto.lib", "Vendor/openssl/lib/x64/Release/libssl.lib", "Vendor/fmod/lib/x64/fmod_vc.lib" }
        postbuildcommands {
            "{COPY} Vendor/openssl/lib/x64/Release/ossl_static.pdb %{cfg.targetdir}",
            "{COPY} Vendor/fmod/lib/x64/fmod_vc.lib %{cfg.targetdir}"
        }
    files {
        "premake5.lua",
        "**.cpp",
        "**.h"
    }