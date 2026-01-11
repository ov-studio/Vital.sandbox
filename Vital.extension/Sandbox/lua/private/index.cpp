/*----------------------------------------------------------------
     Resource: Vital.extension
     Script: Sandbox: private: lua.cpp
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Sandbox Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.extension/Sandbox/lua/public/index.h>
#include <Vital.sandbox/Sandbox/lua/public/api.h>
#include <Vital.sandbox/vital.hpp>

#include <Vital.extension/Sandbox/lua/api/public/ssr.h>
#include <Vital.extension/Sandbox/lua/api/public/ssao.h>
#include <Vital.extension/Sandbox/lua/api/public/ssil.h>
#include <Vital.extension/Sandbox/lua/api/public/sdfgi.h>
#include <Vital.extension/Sandbox/lua/api/public/emissive.h>
#include <Vital.extension/Sandbox/lua/api/public/fog.h>
#include <Vital.extension/Sandbox/lua/api/public/volumetric_fog.h>
#include <Vital.extension/Sandbox/lua/api/public/adjustment.h>

#include <Vital.extension/Engine/public/canvas.h>
#include <Vital.extension/Engine/public/rendertarget.h>
#include <Vital.extension/Engine/public/texture.h>


/////////////////////////////////
// Vital: Godot: Sandbox: Lua //
/////////////////////////////////

namespace Vital::Godot::Sandbox::Lua {
    Vital::Sandbox::Lua::create* vm = nullptr;

    // TODO: REMOVE LATER
    godot::Ref<godot::FontFile> font;
    Vital::Godot::Texture* tex;
    Vital::Godot::Texture* svg;
    Vital::Godot::RenderTarget* rt = nullptr;
    //
    
    Singleton* Singleton::fetch() {
        instance = instance ? instance : memnew(Singleton());
        return instance;
    }

    Singleton::Singleton() {
        godot::UtilityFunctions::print("Initialized Lua vm");
        vm = new Vital::Sandbox::Lua::create({
            {API::SSR::bind, API::SSR::inject},
            {API::SSAO::bind, API::SSAO::inject},
            {API::SSIL::bind, API::SSIL::inject},
            {API::SDFGI::bind, API::SDFGI::inject},
            {API::Emissive::bind, API::Emissive::inject},
            {API::Fog::bind, API::Fog::inject},
            {API::VolumetricFog::bind, API::VolumetricFog::inject},
            {API::Adjustment::bind, API::Adjustment::inject}
        });

        /*
        std::string rwString = R"(
            local a = 10
            local b = 20
            print("Can you see this", "again")
            print("SHA256 of Hello", crypto.hash("SHA256", "hello"))
            timer:create(function()
                print("Executing timer")
            end, 2500, 5)
            return a + b
        )";
        vm -> loadString(rwString);
        double result = vm -> getInt(-1);
        godot::UtilityFunctions::print(result);
        */

        /*
        Vital::Tool::Timer([](auto self) -> void {
            godot::UtilityFunctions::print("executed c++ timer");
        }, 5000, 0);
        */

        // Run in another thread
        /*
        Vital::Tool::Thread([=](Vital::Tool::Thread* thread) -> void {
            try {
                std::string url = "https://jsonplaceholder.typicode.com/posts/1";
                std::string response = Vital::System::Rest::get(url);
                godot::UtilityFunctions::print(response.c_str());
            }
            catch(const std::runtime_error& error) { godot::UtilityFunctions::print(error.what()); }
        }).detach();
        */
        
        //auto stuff = Vital::Tool::Crypto::hash("SHA256", "hello");
        //godot::UtilityFunctions::print(stuff.c_str());
        //ClassDB::register_abstract_class<Lua>();

    }

    void Singleton::ready() {
        std::string rwString = R"(
            print("SSAO Enabled: ", ssao.set_enabled(true))
            print("SSAO Intensity: ", ssao.get_Intensity())
            print("Adjustment Enabled:", adjustment.set_enabled(true))
            print("SSAO Enabled: ", ssao.set_enabled(true))
            print("SSAO Intensity: ", ssao.set_Intensity(5))
        )";
        vm -> loadString(rwString);

        font.instantiate();
        font -> load_dynamic_font(to_godot_string(get_directory()) + to_godot_string("/") + to_godot_string("fonts/Roboto-Bold.ttf"));
        tex = Vital::Godot::Texture::create_texture_2d("flower.jpg");
        svg = Vital::Godot::Texture::create_svg("square.svg");
    }

    void Singleton::process(double delta) {
        //godot::UtilityFunctions::print("rendered");
        
        // Lua script to run
        std::string rwString = R"(
            print("processing")
        )";
        //vm -> loadString(rwString);
    }

    void Singleton::draw(Vital::Godot::Canvas* canvas) {
        //godot::UtilityFunctions::print("rendered");
        
        // Lua script to run
        std::string rwString = R"(
            print("drawing")
        )";

        std::string text = u8"Hello from Аниса from Netherland Hello from Аниса from Netherland 2";
        float rotation = std::sin(get_tick()*0.0005)*360.0f;
        canvas -> draw_image({100 + (310)*0, 20}, {300, 300}, tex -> get_texture(), 0, {0.0f, 0.0f}, {1, 1, 1, 0.35});
        canvas -> draw_text(
            text,
            {100 + (310)*0, 20},
            {100 + (310)*0 + 300, 20 + 300},
            font,
            20,
            {1, 1, 1, 1},
            {godot::HORIZONTAL_ALIGNMENT_CENTER, godot::VERTICAL_ALIGNMENT_TOP},
            false,
            true,
            3,
            {0, 0, 0, 1},
            0.0f,
            {0.0f, 0.0f}
        );

        canvas -> draw_image({100 + (310)*1, 20}, {300, 300}, "flower.jpg", 0, {0.0f, 0.0f}, {1, 1, 1, 0.35});
        canvas -> draw_text(
            text,
            {100 + (310)*1, 20},
            {100 + (310)*1 + 300, 20 + 300},
            font,
            20,
            {1, 1, 1, 1},
            {godot::HORIZONTAL_ALIGNMENT_CENTER, godot::VERTICAL_ALIGNMENT_CENTER},
            false,
            true,
            3,
            {0, 0, 0, 1},
            rotation,
            {0.0f, 0.0f}
        );

        if (!rt) {
            rt = Vital::Godot::RenderTarget::create(512, 512, false);
        }

        Vital::Godot::RenderTarget::set_rendertarget(rt, true, true);
        canvas -> draw_image({0, 20}, {300, 300}, "flower.jpg", rotation, {0.0f, 0.0f}, {1, 1, 1, 0.35});
        canvas -> draw_text(
            text,
            {0, 20},
            {0 + 300, 20 + 300},
            font,
            20,
            {1, 1, 1, 1},
            {godot::HORIZONTAL_ALIGNMENT_CENTER, godot::VERTICAL_ALIGNMENT_BOTTOM},
            true,
            false,
            3,
            {0, 0, 0, 1},
            0.0f,
            {0.0f, 0.0f}
        );
        Vital::Godot::RenderTarget::set_rendertarget();

        canvas -> draw_image({100, 500}, {512, 512}, rt);

        canvas -> draw_rectangle(
            {100 + (310)*3, 20}, 
            {300, 300},
            {0, 0, 1, 1},
            6,
            {0, 0, 0, 1},
            rotation, 
            {0.0f, 0.0f}
        );

        canvas -> draw_circle(
            {100 + (310)*4 + 300*0.5, 20 + 300*0.5},
            300*0.5,
            {1, 0, 0, 1},
            6,
            {0, 0, 0, 1},
            0.0f,
            {0.0f, 0.0f}
        );

        canvas -> draw_line(
            {
                {100 + (310)*5, 20},
                {100 + (310)*5.75, 20},
                {100 + (310)*5, 320},
                {100 + (310)*5.75, 320},
            },
            2,
            {0, 1, 0, 1}
        );

        canvas -> draw_polygon(
            {
                {100 + (310)*6, 20},
                {100 + (310)*7, 20},
                {100 + (310)*7.5, 320*0.5},
                {100 + (310)*7, 320},
                {100 + (310)*6, 320},
                {100 + (310)*5.5, 320*0.5}
            },
            {0, 1, 0, 1},
            6,
            {0, 0, 0, 1},
            rotation, 
            {0.0f, 0.0f}
        );


        canvas -> draw_image({622, 500}, {512, 512}, svg -> get_texture());

        //vm -> loadString(rwString);
    }
}