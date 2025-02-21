project "Vital.sandbox"
    language "C++"
    cppdialect "C++17"
    kind "StaticLib"
    targetdir ("../.build/"..outputdir.."/%{prj.name}")
    objdir ("../.output/"..outputdir.."/%{prj.name}")
    includedirs { ".", "Vendor", "Vendor/openssl/include", "Vendor/fmod/include", "Vendor/lua", "Vendor/rapidjson", "Vendor/duktape" }
    files {
        "premake5.lua",
        "**.cpp",
        "**.c",
        "**.hpp",
        "**.h"
    }

    filter "system:windows"
        links { "ws2_32", "winmm", "wbemuuid" }
    filter "configurations:Debug"
        links { "Vendor/openssl/lib/%{cfg.platform}/Debug/libcrypto.lib", "Vendor/openssl/lib/%{cfg.platform}/Debug/libssl.lib", "Vendor/fmod/lib/%{cfg.platform}/fmodL_vc.lib", "ws2_32", "crypt32" }
        postbuildcommands {
            "copy /y ..\\Vital.sandbox\\Vendor\\openssl\\lib\\%{cfg.platform}\\Debug\\ossl_static.pdb $(OutDir)",
            "copy /y ..\\Vital.sandbox\\Vendor\\fmod\\lib\\%{cfg.platform}\\fmodL.dll $(OutDir)"
        }
    filter "configurations:Release"
        links { "Vendor/openssl/lib/%{cfg.platform}/Release/libcrypto.lib", "Vendor/openssl/lib/%{cfg.platform}/Release/libssl.lib", "Vendor/fmod/lib/%{cfg.platform}/fmod_vc.lib", "ws2_32", "crypt32"  }
        postbuildcommands {
            "copy /y ..\\Vital.sandbox\\Vendor\\openssl\\lib\\%{cfg.platform}\\Release\\ossl_static.pdb $(OutDir)",
            "copy /y ..\\Vital.sandbox\\Vendor\\fmod\\lib\\%{cfg.platform}\\fmod.dll $(OutDir)"
        }