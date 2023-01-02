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
            "copy /y ..\\Vital.sandbox\\Vendor\\openssl\\lib\\x64\\Debug\\ossl_static.pdb $(OutDir)",
            "copy /y ..\\Vital.sandbox\\Vendor\\fmod/lib\\x64\\fmodL_vc.lib $(OutDir)"
        }
    filter "configurations:Release"
        links { "Vendor/openssl/lib/x64/Release/libcrypto.lib", "Vendor/openssl/lib/x64/Release/libssl.lib", "Vendor/fmod/lib/x64/fmod_vc.lib" }
        postbuildcommands {
            "copy /y ..\\Vital.sandbox\\Vendor\\openssl\\lib\\x64\\Release\\ossl_static.pdb $(OutDir)",
            "copy /y ..\\Vital.sandbox\\Vendor\\fmod/lib\\x64\\fmod_vc.lib $(OutDir)"
        }
    files {
        "premake5.lua",
        "**.cpp",
        "**.h"
    }