/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: index.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Sandbox/api/engine.h>
#include <Vital.sandbox/Sandbox/api/database.h>
#include <Vital.sandbox/Sandbox/api/discord.h>
#include <Vital.sandbox/Sandbox/api/coroutine.h>
#include <Vital.sandbox/Sandbox/api/file.h>
#include <Vital.sandbox/Sandbox/api/network.h>
#include <Vital.sandbox/Sandbox/api/rest.h>
#include <Vital.sandbox/Sandbox/api/crypto.h>
#include <Vital.sandbox/Sandbox/api/shrinker.h>
#include <Vital.sandbox/Sandbox/api/canvas.h>
#include <Vital.sandbox/Sandbox/api/model.h>
#include <Vital.sandbox/Sandbox/api/webview.h>
#include <Vital.sandbox/Sandbox/api/font.h>
#include <Vital.sandbox/Sandbox/api/texture.h>
#include <Vital.sandbox/Sandbox/api/rendertarget.h>
#include <Vital.sandbox/Sandbox/api/gfx/ssr.h>
#include <Vital.sandbox/Sandbox/api/gfx/ssao.h>
#include <Vital.sandbox/Sandbox/api/gfx/ssil.h>
#include <Vital.sandbox/Sandbox/api/gfx/sdfgi.h>
#include <Vital.sandbox/Sandbox/api/gfx/emissive.h>
#include <Vital.sandbox/Sandbox/api/gfx/fog.h>
#include <Vital.sandbox/Sandbox/api/gfx/volumetric_fog.h>
#include <Vital.sandbox/Sandbox/api/gfx/adjustment.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::internal_apis = {
        vm_module::make_api<API::Engine>(),
        vm_module::make_api<API::DatabaseQuery>(),
        vm_module::make_api<API::Database>(),
        vm_module::make_api<API::Discord>(),
        vm_module::make_api<API::Coroutine>(),
        vm_module::make_api<API::Network>(),
        vm_module::make_api<API::Rest>(),
        vm_module::make_api<API::File>(),
        vm_module::make_api<API::Crypto>(),
        vm_module::make_api<API::Shrinker>(),


        // TODO: Improve, move under 'API' folder
        vm_module::make_api<API::Canvas>(),
        vm_module::make_api<API::Model>(),
        vm_module::make_api<API::Webview>(),
        vm_module::make_api<API::Font>(),
        vm_module::make_api<API::Texture>(),
        vm_module::make_api<API::SVG>(),
        vm_module::make_api<API::Rendertarget>(),
        vm_module::make_api<API::SSAO>(),
        vm_module::make_api<API::SSR>(),
        vm_module::make_api<API::SSIL>(),
        vm_module::make_api<API::SDFGI>(),
        vm_module::make_api<API::Emissive>(),
        vm_module::make_api<API::Fog>(),
        vm_module::make_api<API::Volumetric_Fog>(),
        vm_module::make_api<API::Adjustment>()
    };

    namespace API {
        void log(const std::string& type, const std::string& message) {
            Vital::print(type, message);
        }
    
        void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
            vm -> bind(scope, name, std::move(exec));
        }
    }
}