/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: shader.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 22/08/2026
     Desc: Shader Utilities
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/shader.h>


/////////////////////////////
// Vital: Engine: Shader  //
/////////////////////////////

// TODO: Improve

namespace Vital::Engine {

    // Managers //
    Shader* Shader::create(const std::string& code, Type type) {
        auto* instance = new Shader();
        instance -> shader_type = type;
        instance -> gd_shader.instantiate();
        instance -> gd_material.instantiate();

        // Inject a shader_type declaration if the user omitted it, so
        // minimal bodies like "void fragment() { COLOR = vec4(1,0,0,1); }"
        // just work without boilerplate.
        std::string src = code;
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (type == Type::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }

        instance -> gd_shader -> set_code(godot::String(src.c_str()));
        instance -> gd_material -> set_shader(instance -> gd_shader);
        return instance;
    }

    void Shader::destroy() {
        gd_material.unref();
        gd_shader.unref();
        delete this;
    }


    // Checkers //
    bool Shader::is_valid() const {
        return gd_shader.is_valid() && !gd_shader -> get_code().is_empty();
    }


    // Getters //
    Shader::Type Shader::get_type() const { return shader_type; }

    std::string Shader::get_code() const {
        if (!gd_shader.is_valid()) return {};
        return std::string(gd_shader -> get_code().utf8().get_data());
    }

    godot::Ref<godot::ShaderMaterial> Shader::get_material() const {
        return gd_material;
    }


    // Setters //
    bool Shader::set_code(const std::string& code) {
        if (!gd_shader.is_valid()) return false;
        std::string src = code;
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (shader_type == Type::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }
        gd_shader -> set_code(godot::String(src.c_str()));
        return true;
    }

    bool Shader::set_param(const std::string& name, const godot::Variant& value) {
        if (!gd_material.is_valid()) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), value);
        return true;
    }

    bool Shader::set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture) {
        if (!gd_material.is_valid()) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }

    bool Shader::set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture) {
        if (!gd_material.is_valid()) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }


    // Misc //
    int Shader::apply_to_node(godot::Node* node) {
        if (!node || !gd_material.is_valid()) return 0;
        int count = 0;
        if (auto* mesh = godot::Object::cast_to<godot::MeshInstance3D>(node)) {
            int surfaces = mesh -> get_surface_override_material_count();
            if (surfaces == 0 && mesh -> get_mesh().is_valid())
                surfaces = mesh -> get_mesh() -> get_surface_count();
            for (int i = 0; i < surfaces; i++) {
                mesh -> set_surface_override_material(i, gd_material);
                count++;
            }
        }
        for (int i = 0; i < node -> get_child_count(); i++)
            count += apply_to_node(node -> get_child(i));
        return count;
    }

}
#endif
