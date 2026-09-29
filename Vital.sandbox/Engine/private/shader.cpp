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
        if (mode == Shader::Mode::Postprocess) {
            // Postprocess shaders run as spatial shaders on a full-screen quad
            // parented to the camera. We inject the required render_mode flags
            // automatically so authors only write the fragment() body and can
            // sample DEPTH_TEXTURE, SCREEN_TEXTURE, NORMAL_ROUGHNESS_TEXTURE, etc.
            if (src.find("shader_type") == std::string::npos)
                src = "shader_type spatial;\n" + src;
            // Inject render_mode block after the shader_type line if not already present
            if (src.find("render_mode") == std::string::npos) {
                auto st_end = src.find(';');
                if (st_end != std::string::npos)
                    src.insert(st_end + 1,
                        "\nrender_mode unshaded, cull_disabled, depth_test_disabled, depth_draw_never;\n");
            }
            return inject_sentinel(src, true);
        }
        if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (mode == Shader::Mode::Spatial) ? "spatial" : "canvas_item";
            src = "shader_type " + type_name + ";\n" + src;
        }
        return inject_sentinel(src, mode == Shader::Mode::Spatial);
    }
}

namespace Vital::Engine {
    // Draw_Clone_Pool //

    godot::Ref<godot::ShaderMaterial> Shader::Draw_Clone_Pool::next(
        godot::Ref<godot::Shader> shader,
        godot::Ref<godot::ShaderMaterial> source)
    {
        uint64_t current_frame = godot::Engine::get_singleton()->get_process_frames();
        if (current_frame != last_frame) {
            used = 0;
            last_frame = current_frame;
        }
        if (used >= clones.size()) {
            godot::Ref<godot::ShaderMaterial> clone;
            clone.instantiate();
            clone->set_shader(shader);
            clones.push_back(clone);
        }
        auto& clone = clones[used++];
        auto list = shader->get_shader_uniform_list();
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            godot::StringName name = static_cast<godot::String>(dict.get("name", godot::String()));
            std::string sname = Tool::to_std_string(godot::String(name));
            if (sname == Shader::Internal::SENTINEL) continue;
            godot::Variant v = source->get_shader_parameter(name);
            if (v.get_type() != godot::Variant::NIL)
                clone->set_shader_parameter(name, v);
        }
        return clone;
    }


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
        instance -> postprocess_init();
        return instance;
    }

    void Shader::destroy() {
        postprocess_free();
        draw_pool.clear();
        surface_materials.clear();
        material.unref();
        shader.unref();
        delete this;
    }



    // Postprocess //

    // Builds a minimal full-screen quad (a QuadMesh covering [-1,1] x [-1,1] in
    // local space) and parents it to the active Camera3D.  The mesh sits just
    // past the near plane so it is always in front of everything, but its
    // render_mode flags (unshaded, depth_test_disabled, depth_draw_never,
    // cull_disabled — injected by build_source) mean it never occludes real
    // geometry and never writes to the depth buffer.
    Shader::Postprocess_Quad Shader::postprocess_make_quad() {
        // Each quad gets its own ShaderMaterial clone so it can hold independent
        // param values from other quads in the same pool.
        Postprocess_Quad pq;
        pq.material.instantiate();
        pq.material->set_shader(shader);

        godot::Ref<godot::QuadMesh> mesh;
        mesh.instantiate();
        mesh->set_size(godot::Vector2(2.0f, 2.0f));

        pq.node = memnew(godot::MeshInstance3D);
        pq.node->set_mesh(mesh);
        pq.node->set_surface_override_material(0, pq.material);
        pq.node->set_position(godot::Vector3(0.0f, 0.0f,
            -(postprocess_camera->get_near() + 0.001f)));
        pq.node->set_cast_shadows_setting(
            godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
        pq.node->set_gi_mode(godot::GeometryInstance3D::GI_MODE_DISABLED);
        pq.node->set_visible(false);
        postprocess_camera->add_child(pq.node);
        return pq;
    }

    void Shader::postprocess_init() {
        if (mode != Mode::Postprocess) return nullptr;
        // Don't cache the camera here — it may change at runtime.
        // draw_postprocess() resolves and tracks it lazily each frame.
    }

    void Shader::postprocess_free() {
        for (auto& pq : postprocess_pool) {
            if (pq.node && pq.node->is_inside_tree())
                pq.node->get_parent()->remove_child(pq.node);
            if (pq.node) memdelete(pq.node);
            if (pq.sub_viewport && pq.sub_viewport->is_inside_tree())
                pq.sub_viewport->get_parent()->remove_child(pq.sub_viewport);
            if (pq.sub_viewport) memdelete(pq.sub_viewport);
            // sub_camera is a child of sub_viewport, freed with it
        }
        postprocess_pool.clear();
        postprocess_used = 0;
        postprocess_camera = nullptr;
    }

    // Lazily create (or resize) the SubViewport + Camera3D for a non-fullscreen draw.
    void Shader::postprocess_ensure_rt(Postprocess_Quad& pq, godot::Vector2 size) {
        auto* tree = godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop());
        if (!tree) return;

        godot::Vector2i isize = godot::Vector2i((int)size.x, (int)size.y);

        if (!pq.sub_viewport) {
            // Move the quad node from the scene camera into its own SubViewport.
            if (pq.node->is_inside_tree())
                pq.node->get_parent()->remove_child(pq.node);

            pq.sub_viewport = memnew(godot::SubViewport);
            pq.sub_viewport->set_size(isize);
            pq.sub_viewport->set_transparent_background(true);
            pq.sub_viewport->set_update_mode(godot::SubViewport::UPDATE_ALWAYS);
            pq.sub_viewport->set_disable_3d(false);

            // Mirror the scene camera so depth/normals match
            pq.sub_camera = memnew(godot::Camera3D);
            pq.sub_viewport->add_child(pq.sub_camera);
            pq.sub_viewport->add_child(pq.node);
            tree->get_root()->add_child(pq.sub_viewport);
        } else if (pq.sub_viewport->get_size() != isize) {
            pq.sub_viewport->set_size(isize);
        }

        // Sync sub_camera to match the active scene camera each frame
        if (postprocess_camera) {
            pq.sub_camera->set_global_transform(postprocess_camera->get_global_transform());
            pq.sub_camera->set_fov(postprocess_camera->get_fov());
            pq.sub_camera->set_near(postprocess_camera->get_near());
            pq.sub_camera->set_far(postprocess_camera->get_far());
        }
    }

    godot::Ref<godot::ViewportTexture> Shader::draw_postprocess(godot::Vector2 position, godot::Vector2 size) {
        if (mode != Mode::Postprocess) return;

        auto* tree = godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop());
        if (!tree) return nullptr;
        auto* current_camera = tree->get_root()->get_camera_3d();
        if (!current_camera) return nullptr;

        // Track camera changes — re-parent fullscreen quads if camera switches
        if (current_camera != postprocess_camera) {
            postprocess_camera = current_camera;
            float cam_near = postprocess_camera->get_near();
            for (auto& pq : postprocess_pool) {
                if (!pq.sub_viewport) {
                    // Only re-parent fullscreen quads (RT quads live in sub_viewport)
                    if (pq.node->is_inside_tree())
                        pq.node->get_parent()->remove_child(pq.node);
                    pq.node->set_position(godot::Vector3(0.0f, 0.0f, -(cam_near + 0.001f)));
                    postprocess_camera->add_child(pq.node);
                }
            }
        }

        uint64_t current_frame = godot::Engine::get_singleton()->get_process_frames();

        // New frame — reset pool cursor and hide all quads from last frame
        if (current_frame != postprocess_last_frame) {
            postprocess_last_frame = current_frame;
            for (size_t i = 0; i < postprocess_used; i++)
                postprocess_pool[i].node->set_visible(false);
            postprocess_used = 0;
        }

        // Get or allocate a quad for this draw call
        if (postprocess_used >= postprocess_pool.size())
            postprocess_pool.push_back(postprocess_make_quad());

        auto& pq = postprocess_pool[postprocess_used++];

        // Stamp current param state into this quad's own material clone
        auto list = shader->get_shader_uniform_list();
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            godot::StringName name = static_cast<godot::String>(dict.get("name", godot::String()));
            std::string sname = Tool::to_std_string(godot::String(name));
            if (sname == Internal::SENTINEL) continue;
            godot::Variant v = material->get_shader_parameter(name);
            if (v.get_type() != godot::Variant::NIL)
                pq.material->set_shader_parameter(name, v);
        }

        bool sized = size.x > 0.0f && size.y > 0.0f;
        if (sized) {
            // RT path — render quad into SubViewport, return texture for canvas to blit
            postprocess_ensure_rt(pq, size);
            pq.node->set_visible(true);
            return pq.sub_viewport->get_texture();
        } else {
            // Fullscreen path — show quad directly, return nullptr (no blit needed)
            if (pq.sub_viewport) {
                // Quad was previously RT — move back to scene camera
                if (pq.node->is_inside_tree())
                    pq.node->get_parent()->remove_child(pq.node);
                float cam_near = postprocess_camera->get_near();
                pq.node->set_position(godot::Vector3(0.0f, 0.0f, -(cam_near + 0.001f)));
                postprocess_camera->add_child(pq.node);
                pq.sub_viewport->set_update_mode(godot::SubViewport::UPDATE_DISABLED);
            }
            pq.node->set_visible(true);
            return nullptr;
        }
    }



    godot::Ref<godot::ShaderMaterial> Shader::snapshot_draw_material() {
        if (mode != Mode::CanvasItem) return material;
        return draw_pool.next(shader, material);
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
