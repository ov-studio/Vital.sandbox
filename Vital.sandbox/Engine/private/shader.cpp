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
#include <algorithm>
#include <cctype>


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
                { "void fragment()", "ROUGHNESS *= " + std::string(SENTINEL) + ";\n" },
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

    std::string Shader::Internal::build_source(const std::string& raw, Shader::Mode mode) {
        std::string src = raw;
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (mode == Shader::Mode::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }
        return inject_sentinel(src, mode == Shader::Mode::Spatial);
    }
}

namespace Vital::Engine {
    // Managers //
    Shader* Shader::create(const std::string& base, const std::string& path, Mode mode) {
        return create_from_raw(Tool::File::read_text(base, path), mode);
    }

    Shader* Shader::create_from_raw(const std::string& raw, Mode mode) {
        auto instance = new Shader();
        instance -> mode = mode;
        instance -> shader.instantiate();
        instance -> material.instantiate();
        instance -> shader -> set_code(Tool::to_godot_string(Internal::build_source(raw, instance -> mode)));
        if (!Internal::validate_compiled(instance -> shader)) {
            delete instance;
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "shader failed to compile");
        }
        instance -> material -> set_shader(instance -> shader);
        return instance;
    }

    void Shader::destroy() {
        surface_materials.clear();
        material.unref();
        shader.unref();
        delete this;
    }


    // Getters //
    Shader::Mode Shader::get_mode() const { 
        return mode;
    }

    std::string Shader::get_code() const {
        return std::string(shader -> get_code().utf8().get_data());
    }

    godot::Ref<godot::ShaderMaterial> Shader::get_material() const {
        return material;
    }

    void Shader::prune_surface_materials() {
        surface_materials.erase(
            std::remove_if(surface_materials.begin(), surface_materials.end(),
                [](const godot::Ref<godot::ShaderMaterial>& m) { return !m.is_valid() || m -> get_reference_count() <= 1; }),
            surface_materials.end()
        );
    }

    // Maps a sampler uniform name to the BaseMaterial3D texture slot it should
    // inherit from the mesh's original material. -1 = no automatic source.
    static int texture_slot_for_uniform(const std::string& raw) {
        std::string n = raw;
        std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        auto has = [&](const char* s) { return n.find(s) != std::string::npos; };
        if (has("albedo") || has("diffuse") || has("basecolor") || has("base_color")) return godot::BaseMaterial3D::TEXTURE_ALBEDO;
        if (has("normal"))    return godot::BaseMaterial3D::TEXTURE_NORMAL;
        if (has("rough"))     return godot::BaseMaterial3D::TEXTURE_ROUGHNESS;
        if (has("metal"))     return godot::BaseMaterial3D::TEXTURE_METALLIC;
        if (has("emission"))  return godot::BaseMaterial3D::TEXTURE_EMISSION;
        if (has("occlusion") || n == "ao_texture" || has("ambient")) return godot::BaseMaterial3D::TEXTURE_AMBIENT_OCCLUSION;
        return -1;
    }

    // Builds a per-surface clone of the shader material and feeds it the
    // textures of the surface's ORIGINAL material. This is what makes
    // `texture(albedo_texture, UV)` return the model's real texture instead of
    // the 1x1 white fallback of `hint_default_white`.
    godot::Ref<godot::ShaderMaterial> Shader::create_surface_material(godot::Ref<godot::Material> original) {
        godot::Ref<godot::ShaderMaterial> clone;
        clone.instantiate();
        clone -> set_shader(shader);
        prune_surface_materials();

        godot::Ref<godot::BaseMaterial3D> base = original;
        auto list = shader -> get_shader_uniform_list();
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            godot::StringName name = static_cast<godot::String>(dict.get("name", godot::String()));
            std::string std_name = Tool::to_std_string(godot::String(name));
            if (std_name == Internal::SENTINEL) continue;

            // 1. Inherit whatever the user set on the shared material (tints, floats, textures...)
            godot::Variant value = material -> get_shader_parameter(name);
            if (value.get_type() != godot::Variant::NIL) {
                clone -> set_shader_parameter(name, value);
                continue;
            }

            // 2. Otherwise auto-bind the original material's values for matching uniform names
            if (mode != Mode::Spatial || !base.is_valid()) continue;
            int type = static_cast<int>(dict.get("type", 0));

            if (type == godot::Variant::OBJECT) {
                int slot = texture_slot_for_uniform(std_name);
                if (slot < 0) continue;
                godot::Ref<godot::Texture2D> tex = base -> get_texture(static_cast<godot::BaseMaterial3D::TextureParam>(slot));
                if (tex.is_valid()) clone -> set_shader_parameter(name, tex);
                continue;
            }

            std::string n = std_name;
            std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            godot::Variant v;
            if      (n == "albedo_color" || n == "albedo") {
                godot::Color c = base -> get_albedo();
                if (type == godot::Variant::COLOR) v = c;
                else if (type == godot::Variant::VECTOR4) v = godot::Vector4(c.r, c.g, c.b, c.a);
                else if (type == godot::Variant::VECTOR3) v = godot::Vector3(c.r, c.g, c.b);
            }
            else if (n == "roughness")               v = base -> get_roughness();
            else if (n == "metallic")                v = base -> get_metallic();
            else if (n == "specular")                v = base -> get_specular();
            else if (n == "normal_scale")            v = base -> get_normal_scale();
            else if (n == "alpha_scissor_threshold") v = base -> get_alpha_scissor_threshold();
            else if (n == "emission_energy")         v = base -> get_emission_energy_multiplier();
            else if (n == "emission" || n == "emission_color") {
                godot::Color c = base -> get_emission();
                if (type == godot::Variant::COLOR) v = c;
                else if (type == godot::Variant::VECTOR4) v = godot::Vector4(c.r, c.g, c.b, c.a);
                else if (type == godot::Variant::VECTOR3) v = godot::Vector3(c.r, c.g, c.b);
            }
            else if (n == "roughness_channel" || n == "metallic_channel" || n == "ao_channel") {
                // Which texture channel Godot reads for this property (glTF ORM: ao=R, roughness=G, metallic=B).
                // Exposed as a vec4 mask: use dot(texture(tex, uv), mask).
                int ch = (n == "roughness_channel") ? (int)base -> get_roughness_texture_channel()
                       : (n == "metallic_channel")  ? (int)base -> get_metallic_texture_channel()
                                                    : (int)base -> get_ao_texture_channel();
                godot::Vector4 mask(1, 0, 0, 0);
                switch (ch) {
                    case godot::BaseMaterial3D::TEXTURE_CHANNEL_GREEN:     mask = godot::Vector4(0, 1, 0, 0); break;
                    case godot::BaseMaterial3D::TEXTURE_CHANNEL_BLUE:      mask = godot::Vector4(0, 0, 1, 0); break;
                    case godot::BaseMaterial3D::TEXTURE_CHANNEL_ALPHA:     mask = godot::Vector4(0, 0, 0, 1); break;
                    case godot::BaseMaterial3D::TEXTURE_CHANNEL_GRAYSCALE: mask = godot::Vector4(0.333333f, 0.333333f, 0.333333f, 0); break;
                    default: break;
                }
                if (type == godot::Variant::COLOR) v = godot::Color(mask.x, mask.y, mask.z, mask.w);
                else v = mask;
            }
            else if (n == "uv1_scale")  v = base -> get_uv1_scale();
            else if (n == "uv1_offset") v = base -> get_uv1_offset();
            if (v.get_type() != godot::Variant::NIL) clone -> set_shader_parameter(name, v);
        }
        surface_materials.push_back(clone);
        return clone;
    }

    Shader::SurfaceFactory Shader::get_surface_factory() {
        return [this](godot::Ref<godot::Material> original) { return create_surface_material(original); };
    }


    // Setters //
    bool Shader::set_param(const std::string& name, const godot::Variant& value) {
        if (!material.is_valid()) return false;
        if (name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), value);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(name.c_str()), value);
        return true;
    }

    bool Shader::set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }

    bool Shader::set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(name.c_str()), texture);
        return true;
    }
}
#endif
