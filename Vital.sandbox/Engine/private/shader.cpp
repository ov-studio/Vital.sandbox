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
    static constexpr const char* SENTINEL_NAME = "_vsdk_sentinel";

    // TODO: Add these to internals and expose in header too
    static std::string inject_sentinel(const std::string& src, bool is_spatial) {
        const std::string decl =
            "uniform float " + std::string(SENTINEL_NAME) +
            " : hint_range(1.0, 1.0) = 1.0;\n";

        struct EntryPoint {
            std::string signature;
            std::string usage;
        };

        const std::vector<EntryPoint> entry_points = is_spatial
            ? std::vector<EntryPoint>{
                { "void fragment()", "ALPHA *= " + std::string(SENTINEL_NAME) + ";\n" },
                { "void vertex()",   "VERTEX *= " + std::string(SENTINEL_NAME) + ";\n" } }
            : std::vector<EntryPoint>{
                { "void fragment()", "COLOR.a *= " + std::string(SENTINEL_NAME) + ";\n" },
                { "void vertex()",   "VERTEX *= " + std::string(SENTINEL_NAME) + ";\n" } };

        std::string result = src;
        auto st_end = result.find(';');
        if (st_end != std::string::npos) result.insert(st_end + 1, "\n" + decl);
        else result = decl + result;
        for (const auto& ep : entry_points) {
            std::size_t pos = 0;
            while ((pos = result.find(ep.signature, pos)) != std::string::npos) {
                auto brace = result.find('{', pos + ep.signature.size());
                if (brace == std::string::npos) break;
                result.insert(brace + 1, "\n" + ep.usage);
                pos = brace + 1 + ep.usage.size();
            }
        }
        return result;
    }

    static bool validate_compiled(godot::Ref<godot::Shader>& gd_shader) {
        auto list = gd_shader -> get_shader_uniform_list();
        bool sentinel_found = false;
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            auto name = static_cast<godot::String>(dict.get("name", godot::String()));
            if (std::string(name.utf8().get_data()) == SENTINEL_NAME) {
                sentinel_found = true;
                break;
            }
        }
        if (!sentinel_found) return false;
        return true;
    }

    static std::string build_source(const std::string& code, Shader::Type type) {
        std::string src = code;
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (type == Shader::Type::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }
        return inject_sentinel(src, type == Shader::Type::Spatial);
    }


    // Managers //
    Shader* Shader::create(const std::string& code, Type type) {
        auto* instance = new Shader();
        instance -> shader_type = type;
        instance -> gd_shader.instantiate();
        instance -> gd_material.instantiate();
        auto src = build_source(code, type);
        instance -> gd_shader -> set_code(godot::String(src.c_str()));
        instance -> gd_material -> set_shader(instance -> gd_shader);
        if (!validate_compiled(instance -> gd_shader)) {
            delete instance;
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "shader failed to compile");
        }
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
    Shader::Type Shader::get_type() const { 
        return shader_type;
    }

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
        auto src = build_source(code, shader_type);
        gd_shader -> set_code(godot::String(src.c_str()));
        return validate_compiled(gd_shader);
    }

    bool Shader::set_param(const std::string& name, const godot::Variant& value) {
        if (!gd_material.is_valid()) return false;
        if (name == SENTINEL_NAME) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), value);
        return true;
    }

    bool Shader::set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture) {
        if (!gd_material.is_valid() || name == SENTINEL_NAME) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }

    bool Shader::set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture) {
        if (!gd_material.is_valid() || name == SENTINEL_NAME) return false;
        gd_material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }


    // Misc //
    int Shader::apply_to_node(godot::Node* node) {
        if (!node || !gd_material.is_valid()) return 0;
        int count = 0;
        if (auto* mesh = godot::Object::cast_to<godot::MeshInstance3D>(node)) {
            int surfaces = mesh -> get_surface_override_material_count();
            if (surfaces == 0 && mesh -> get_mesh().is_valid()) surfaces = mesh -> get_mesh() -> get_surface_count();
            for (int i = 0; i < surfaces; i++) {
                mesh -> set_surface_override_material(i, gd_material);
                count++;
            }
        }
        for (int i = 0; i < node -> get_child_count(); i++) count += apply_to_node(node -> get_child(i));
        return count;
    }
}
#endif