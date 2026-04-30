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
#include <Vital.sandbox/Sandbox/machine.h>
#include <Vital.sandbox/Engine/public/console.h>
#include <Vital.sandbox/API/core/engine.h>
#include <Vital.sandbox/API/core/database.h>
#include <Vital.sandbox/API/core/discord.h>
#include <Vital.sandbox/API/core/network.h>
#include <Vital.sandbox/API/core/canvas.h>
#include <Vital.sandbox/API/core/model.h>
#include <Vital.sandbox/API/core/webview.h>
#include <Vital.sandbox/API/core/font.h>
#include <Vital.sandbox/API/core/texture.h>
#include <Vital.sandbox/API/core/rendertarget.h>
//#include <Vital.sandbox/API/utility/coroutine.h>
#include <Vital.sandbox/API/utility/timer.h>
//#include <Vital.sandbox/API/utility/thread.h>
#include <Vital.sandbox/API/utility/file.h>
#include <Vital.sandbox/API/utility/rest.h>
#include <Vital.sandbox/API/utility/crypto.h>
#include <Vital.sandbox/API/utility/shrinker.h>
#include <Vital.sandbox/API/gfx/ssr.h>
#include <Vital.sandbox/API/gfx/ssao.h>
#include <Vital.sandbox/API/gfx/ssil.h>
#include <Vital.sandbox/API/gfx/sdfgi.h>
#include <Vital.sandbox/API/gfx/fog.h>
#include <Vital.sandbox/API/gfx/volumetric_fog.h>
#include <Vital.sandbox/API/gfx/emissive.h>
#include <Vital.sandbox/API/gfx/adjustment.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox {
    vm_apis Machine::internal_apis = {
        // TODO: Improve, move under 'API' namespace??
        vm_module::make_api<API::Engine>(),
        vm_module::make_api<API::DatabaseQuery>(),
        vm_module::make_api<API::Database>(),
        vm_module::make_api<API::Discord>(),
        vm_module::make_api<API::Network>(),
        //vm_module::make_api<API::Coroutine>(),
        vm_module::make_api<API::Timer>(),
        //vm_module::make_api<API::Thread>(),
        vm_module::make_api<API::File>(),
        vm_module::make_api<API::Rest>(),
        vm_module::make_api<API::Crypto>(),
        vm_module::make_api<API::Shrinker>(),
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
        vm_module::make_api<API::Fog>(),
        vm_module::make_api<API::Volumetric_Fog>(),
        vm_module::make_api<API::Emissive>(),
        vm_module::make_api<API::Adjustment>()
    };

    namespace API {
        void log(const std::string& type, const std::string& message) {
            Tool::print(type, message);
        }
    
        void bind(Machine* vm, const std::vector<std::string>& scope, const std::string& name, vm_bind exec) {
            vm -> bind(scope, name, std::move(exec));
        }
    }
}