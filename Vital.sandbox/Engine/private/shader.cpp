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


////////////////////////////
// Vital: Engine: Shader //
////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    std::string Shader::Internal::inject_sentinel(const std::string& src, bool is_spatial) {
        const std::string decl =
            "uniform float " + std::string(SENTINEL) +
            " : hint_range(1.0, 1.0) = 1.0;\n";

        const std::vector<EntryPoint> entry_points = is_spatial ? 
            std::vector<EntryPoint>{
                { "void fragment()", "ALPHA *= " + std::string(SENTINEL) + ";\n" },
                { "void vertex()",   "VERTEX *= " + std::string(SENTINEL) + ";\n" } 
            } : 
            std::vector<EntryPoint>{
                { "void fragment()", "COLOR.a *= " + std::string(SENTINEL) + ";\n" },
                { "void vertex()",   "VERTEX *= " + std::string(SENTINEL) + ";\n" } 
            };

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

    bool Shader::Internal::validate_compiled(godot::Ref<godot::Shader>& shader) {
        auto list = shader -> get_shader_uniform_list();
        bool sentinel_found = false;
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            auto name = static_cast<godot::String>(dict.get("name", godot::String()));
            if (std::string(name.utf8().get_data()) == SENTINEL) {
                sentinel_found = true;
                break;
            }
        }
        if (!sentinel_found) return false;
        return true;
    }

    std::string Shader::Internal::build_source(const std::string& code, Shader::Type type) {
        std::string src = code;
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (type == Shader::Type::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }
        return inject_sentinel(src, type == Shader::Type::Spatial);
    }
}

namespace Vital::Engine {
    // Managers //
    Shader* Shader::create(const std::string& code, Type type) {
        auto* instance = new Shader();
        instance -> shader_type = type;
        instance -> shader.instantiate();
        instance -> material.instantiate();
        auto src = Internal::build_source(code, type);
        instance -> shader -> set_code(godot::String(src.c_str()));
        instance -> material -> set_shader(instance -> shader);
        if (!Internal::validate_compiled(instance -> shader)) {
            delete instance;
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "shader failed to compile");
        }
        return instance;
    }

    void Shader::destroy() {
        material.unref();
        shader.unref();
        delete this;
    }


    // Checkers //
    bool Shader::is_valid() const {
        return shader.is_valid() && !shader -> get_code().is_empty();
    }


    // Getters //
    Shader::Type Shader::get_type() const { 
        return shader_type;
    }

    std::string Shader::get_code() const {
        if (!shader.is_valid()) return {};
        return std::string(shader -> get_code().utf8().get_data());
    }

    godot::Ref<godot::ShaderMaterial> Shader::get_material() const {
        return material;
    }


    // Setters //
    bool Shader::set_code(const std::string& code) {
        if (!shader.is_valid()) return false;
        auto src = Internal::build_source(code, shader_type);
        shader -> set_code(godot::String(src.c_str()));
        return Internal::validate_compiled(shader);
    }

    bool Shader::set_param(const std::string& name, const godot::Variant& value) {
        if (!material.is_valid()) return false;
        if (name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), value);
        return true;
    }

    bool Shader::set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }

    bool Shader::set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }


    // Misc //
    int Shader::apply_to_node(godot::Node* node) {
        if (!node || !material.is_valid()) return 0;
        int count = 0;
        if (auto* mesh = godot::Object::cast_to<godot::MeshInstance3D>(node)) {
            int surfaces = mesh -> get_surface_override_material_count();
            if (surfaces == 0 && mesh -> get_mesh().is_valid()) surfaces = mesh -> get_mesh() -> get_surface_count();
            for (int i = 0; i < surfaces; i++) {
                mesh -> set_surface_override_material(i, material);
                count++;
            }
        }
        for (int i = 0; i < node -> get_child_count(); i++) count += apply_to_node(node -> get_child(i));
        return count;
    }
}
#endif
