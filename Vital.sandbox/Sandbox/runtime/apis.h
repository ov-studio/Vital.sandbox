/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Sandbox: apis.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox APIs
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Sandbox/runtime/module.h>

#include <Vital.sandbox/API/core/engine.h>
#include <Vital.sandbox/API/core/canvas.h>
#include <Vital.sandbox/API/core/database.h>
#include <Vital.sandbox/API/core/database_query.h>
#include <Vital.sandbox/API/core/discord.h>
#include <Vital.sandbox/API/core/model.h>
#include <Vital.sandbox/API/core/webview.h>
#include <Vital.sandbox/API/core/font.h>
#include <Vital.sandbox/API/core/texture.h>
#include <Vital.sandbox/API/core/svg.h>
#include <Vital.sandbox/API/core/rendertarget.h>

#include <Vital.sandbox/API/utility/timer.h>
#include <Vital.sandbox/API/utility/promise.h>
#include <Vital.sandbox/API/utility/thread.h>
#include <Vital.sandbox/API/utility/file.h>
#include <Vital.sandbox/API/utility/http.h>
#include <Vital.sandbox/API/utility/crypto.h>
#include <Vital.sandbox/API/utility/shrinker.h>
#include <Vital.sandbox/API/utility/exports.h>
#include <Vital.sandbox/API/utility/event.h>

#include <Vital.sandbox/API/gfx/adjustment.h>
#include <Vital.sandbox/API/gfx/emissive.h>
#include <Vital.sandbox/API/gfx/ssr.h>
#include <Vital.sandbox/API/gfx/ssil.h>
#include <Vital.sandbox/API/gfx/sdfgi.h>
#include <Vital.sandbox/API/gfx/ssao.h>
#include <Vital.sandbox/API/gfx/fog.h>
#include <Vital.sandbox/API/gfx/volumetric_fog.h>


/////////////////////
// Vital: Sandbox //
/////////////////////

namespace Vital::Sandbox::API {
    inline vm_apis make_apis() {
        return {
            // Core //
            vm_module::make_api<Engine>(),
            vm_module::make_api<Canvas>(),
            vm_module::make_api<Database>(),
            vm_module::make_api<Database_Query>(),
            vm_module::make_api<Discord>(),
            vm_module::make_api<Model>(),
            vm_module::make_api<Webview>(),
            vm_module::make_api<Font>(),
            vm_module::make_api<Texture>(),
            vm_module::make_api<SVG>(),
            vm_module::make_api<Rendertarget>(),

            // Utility //
            vm_module::make_api<Timer>(),
            vm_module::make_api<Promise>(),
            vm_module::make_api<Thread>(),
            vm_module::make_api<File>(),
            vm_module::make_api<HTTP>(),
            vm_module::make_api<Crypto>(),
            vm_module::make_api<Shrinker>(),
            vm_module::make_api<Exports>(),
            vm_module::make_api<Event>(),

            // GFX //
            vm_module::make_api<Adjustment>(),
            vm_module::make_api<Emissive>(),
            vm_module::make_api<SSR>(),
            vm_module::make_api<SSIL>(),
            vm_module::make_api<SDFGI>(),
            vm_module::make_api<SSAO>(),
            vm_module::make_api<Fog>(),
            vm_module::make_api<Volumetric_Fog>()
        };
    }
}