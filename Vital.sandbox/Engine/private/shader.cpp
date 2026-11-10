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
#include <Vital.sandbox/Engine/public/rendertarget.h>


////////////////////////////
// Vital: Engine: Shader //
////////////////////////////

// TODO: Improve
namespace Vital::Engine {
    std::string Shader::Internal::inject_sentinel(const std::string& src, Mode mode) {
        const std::string decl = fmt::format("uniform float {} : hint_range(1.0, 1.0) = 1.0;\n", SENTINEL);
        std::vector<EntryPoint> entry_points;
        if (mode == Mode::Spatial || mode == Mode::Postprocess) {
            entry_points = {
                { "void fragment()", fmt::format("ROUGHNESS *= {};\n", SENTINEL) },
                { "void vertex()",   fmt::format("VERTEX *= {};\n", SENTINEL)    }
            };
        }
        else {
            entry_points = {
                { "void fragment()", fmt::format("COLOR.a *= {};\n", SENTINEL) },
                { "void vertex()",   fmt::format("VERTEX *= {};\n", SENTINEL)  }
            };
        }

        std::string result = src;
        auto st_end = result.find(';');
        if (st_end != std::string::npos) result.insert(st_end + 1, fmt::format("\n{}", decl));
        else result = fmt::format("{}{}", decl, result);
        for (const auto& ep : entry_points) {
            std::size_t pos = 0;
            while ((pos = result.find(ep.signature, pos)) != std::string::npos) {
                auto brace = result.find('{', pos + ep.signature.size());
                if (brace == std::string::npos) break;
                result.insert(brace + 1, fmt::format("\n{}", ep.usage));
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
            if (Tool::to_std_string(name) == SENTINEL) {
                sentinel_found = true;
                break;
            }
        }
        if (!sentinel_found) return false;
        return true;
    }

    std::string Shader::Internal::build_source(const std::string& raw, Shader::Mode mode, const std::string& active_output) {
        std::string src = raw;
        if (mode == Shader::Mode::Postprocess) {
            if (src.find("shader_type") == std::string::npos) src = fmt::format("shader_type spatial;\n{}", src);
            if (src.find("render_mode") == std::string::npos) {
                auto st_end = src.find(';');
                if (st_end != std::string::npos) src.insert(st_end + 1, "\nrender_mode unshaded, cull_disabled, depth_test_disabled, depth_draw_never;\n");
            }
        }
        else if (src.find("shader_type") == std::string::npos) {
            std::string type_name = (mode == Shader::Mode::Spatial) ? "spatial" : "canvas_item";
            src = fmt::format("shader_type {};\n{}", type_name, src);
        }
        if (!active_output.empty() && mode == Shader::Mode::Spatial) src = apply_output_render_modes(src);
        src = rewrite_outputs(src, mode, active_output);
        return inject_sentinel(src, mode);
    }
}


namespace Vital::Engine {
    // Emulated MRT source helpers //
    namespace {
        struct Output_Call {
            size_t begin = 0;
            size_t end = 0;
            std::string name;
            std::string expr;
        };

        bool is_ident_char(char c) {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        }

        std::string trim_copy(const std::string& value) {
            size_t first = 0, last = value.size();
            while (first < last && std::isspace(static_cast<unsigned char>(value[first]))) first++;
            while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1]))) last--;
            return value.substr(first, last - first);
        }

        // The call's trailing `;` is included so the whole statement can be replaced
        std::vector<Output_Call> find_output_calls(const std::string& src) {
            static const std::string token = "VSDK_OUT";
            std::vector<Output_Call> calls;
            size_t i = 0;
            while (i < src.size()) {
                if (src.compare(i, 2, "//") == 0) {
                    auto stop = src.find('\n', i);
                    if (stop == std::string::npos) break;
                    i = stop + 1;
                    continue;
                }
                if (src.compare(i, 2, "/*") == 0) {
                    auto stop = src.find("*/", i + 2);
                    if (stop == std::string::npos) break;
                    i = stop + 2;
                    continue;
                }
                if (src.compare(i, token.size(), token) != 0 || (i > 0 && is_ident_char(src[i - 1]))) {
                    i++;
                    continue;
                }
                size_t j = i + token.size();
                if (j < src.size() && is_ident_char(src[j])) { i = j; continue; }
                while (j < src.size() && std::isspace(static_cast<unsigned char>(src[j]))) j++;
                if (j >= src.size() || src[j] != '(') { i = j; continue; }

                const size_t open = j;
                size_t comma = std::string::npos;
                size_t close = std::string::npos;
                int depth = 1;
                for (size_t k = open + 1; k < src.size(); k++) {
                    const char c = src[k];
                    if (c == '(') depth++;
                    else if (c == ')') {
                        if (--depth == 0) { close = k; break; }
                    }
                    else if (c == ',' && depth == 1 && comma == std::string::npos) comma = k;
                }
                if (close == std::string::npos) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "VSDK_OUT: missing closing ')'");
                if (comma == std::string::npos) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "VSDK_OUT expects (name, value)");

                Output_Call call;
                call.begin = i;
                call.name = trim_copy(src.substr(open + 1, comma - open - 1));
                call.expr = trim_copy(src.substr(comma + 1, close - comma - 1));
                bool valid_name = !call.name.empty() && !std::isdigit(static_cast<unsigned char>(call.name[0]));
                for (char c : call.name) if (!is_ident_char(c)) valid_name = false;
                if (!valid_name) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("VSDK_OUT: invalid output name '{}'", call.name));
                if (call.expr.empty()) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("VSDK_OUT: output '{}' has no value", call.name));

                call.end = close + 1;
                size_t t = call.end;
                while (t < src.size() && std::isspace(static_cast<unsigned char>(src[t]))) t++;
                if (t < src.size() && src[t] == ';') call.end = t + 1;
                calls.push_back(call);
                i = call.end;
            }
            return calls;
        }
    }

    std::vector<std::string> Shader::Internal::scan_outputs(const std::string& raw) {
        std::vector<std::string> names;
        for (const auto& call : find_output_calls(raw)) {
            if (std::find(names.begin(), names.end(), call.name) == names.end()) names.push_back(call.name);
        }
        return names;
    }

    // Output variant: the matching VSDK_OUT stores into a local that fragment() writes last (shaders cannot `return`
    // from fragment()); every other VSDK_OUT, and all of them in the world variant, become empty blocks.
    std::string Shader::Internal::rewrite_outputs(const std::string& src, Mode mode, const std::string& active_output) {
        auto calls = find_output_calls(src);
        if (calls.empty()) return src;

        // Output stays opaque (alpha = coverage) unless the shader writes ALPHA itself
        static const std::regex alpha_pattern(R"(\bALPHA\b)");
        const bool uses_alpha = mode == Mode::Spatial && std::regex_search(src, alpha_pattern);

        std::string result;
        size_t cursor = 0;
        for (const auto& call : calls) {
            result.append(src, cursor, call.begin - cursor);
            if (!active_output.empty() && call.name == active_output) result += fmt::format("{{ vsdk_result = ({}); }}", call.expr);
            else result += "{}";
            cursor = call.end;
        }
        result.append(src, cursor, std::string::npos);
        if (active_output.empty()) return result;

        static const std::regex fragment_pattern(R"(\bvoid\s+fragment\s*\(\s*\)\s*\{)");
        std::smatch match;
        if (!std::regex_search(result, match, fragment_pattern))
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "VSDK_OUT requires a fragment() function");
        const size_t open = static_cast<size_t>(match.position(0) + match.length(0)) - 1;
        size_t close = std::string::npos;
        int depth = 1;
        for (size_t i = open + 1; i < result.size(); i++) {
            if (result.compare(i, 2, "//") == 0) {
                auto stop = result.find('\n', i);
                if (stop == std::string::npos) break;
                i = stop;
                continue;
            }
            if (result.compare(i, 2, "/*") == 0) {
                auto stop = result.find("*/", i + 2);
                if (stop == std::string::npos) break;
                i = stop + 1;
                continue;
            }
            if (result[i] == '{') depth++;
            else if (result[i] == '}' && --depth == 0) { close = i; break; }
        }
        if (close == std::string::npos) throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "fragment() is missing its closing '}'");

        const std::string tail = (mode == Mode::Spatial)
            ? fmt::format("\nALBEDO = vsdk_result.rgb;{}\n", uses_alpha ? " ALPHA = vsdk_result.a;" : "")
            : std::string("\nCOLOR = vsdk_result;\n");
        result.insert(close, tail);
        result.insert(open + 1, "\nvec4 vsdk_result = vec4(0.0);\n");
        return result;
    }

    // Spatial output variants are data passes: unlit, no shadows or fog
    std::string Shader::Internal::apply_output_render_modes(const std::string& src) {
        static const std::regex pattern(R"(\brender_mode\b([^;]*);)");
        const std::vector<std::string> required = { "unshaded", "shadows_disabled", "fog_disabled", "depth_draw_always" };

        std::vector<std::string> modes;
        std::smatch match;
        const bool found = std::regex_search(src, match, pattern);
        if (found) {
            std::stringstream stream(match[1].str());
            std::string item;
            while (std::getline(stream, item, ',')) {
                item = trim_copy(item);
                if (item.empty()) continue;
                if (item.rfind("blend_", 0) == 0 || item.rfind("depth_draw_", 0) == 0 || item.rfind("depth_prepass", 0) == 0) continue;
                if (std::find(required.begin(), required.end(), item) != required.end()) continue;
                modes.push_back(item);
            }
        }
        for (const auto& mode : required) modes.push_back(mode);

        std::string statement = "render_mode ";
        for (size_t i = 0; i < modes.size(); i++) statement += (i ? ", " : "") + modes[i];
        statement += ";";

        std::string result = src;
        if (found) result.replace(static_cast<size_t>(match.position(0)), static_cast<size_t>(match.length(0)), statement);
        else {
            auto st_end = result.find(';');
            if (st_end != std::string::npos) result.insert(st_end + 1, fmt::format("\n{}\n", statement));
        }
        return result;
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
        const auto output_names = Internal::scan_outputs(raw);
        if (!output_names.empty() && mode == Mode::Postprocess)
            throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, "VSDK_OUT outputs are not supported by post_process shaders");

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

        for (const auto& name : output_names) {
            Output output;
            output.name = name;
            output.shader.instantiate();
            output.shader -> set_code(Tool::to_godot_string(Internal::build_source(raw, instance -> mode, name)));
            if (!Internal::validate_compiled(output.shader)) {
                delete instance;
                throw Tool::Log::fetch("request-failed", Tool::Log::Type::error, fmt::format("shader output '{}' failed to compile", name));
            }
            instance -> outputs.push_back(output);
        }
        instance -> postprocess_init();
        return instance;
    }

    void Shader::destroy() {
        postprocess_free();
        draw_pool.clear();
        for (auto& output : outputs) output.pool.clear();
        outputs.clear();
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
    void Shader::postprocess_init() {
        if (mode != Mode::Postprocess) return;
        // Quad created lazily on first draw_postprocess() once camera is known
        postprocess_registry.push_back(this);
    }

    godot::MeshInstance3D* Shader::pp_get_quad() const {
        if (!pp_quad_id.is_valid()) return nullptr;
        return godot::Object::cast_to<godot::MeshInstance3D>(godot::ObjectDB::get_instance(pp_quad_id));
    }

    void Shader::postprocess_free() {
        postprocess_registry.erase(
            std::remove(postprocess_registry.begin(), postprocess_registry.end(), this),
            postprocess_registry.end()
        );
        if (auto* quad = pp_get_quad()) {
            if (auto* parent = quad->get_parent()) parent->remove_child(quad);
            quad->queue_free();
        }
        pp_quad_id = godot::ObjectID();
        pp_material.unref();
    }

    void Shader::draw_postprocess() {
        if (mode != Mode::Postprocess) return;

        auto* tree = godot::Object::cast_to<godot::SceneTree>(
            godot::Engine::get_singleton()->get_main_loop());
        if (!tree) return;
        auto* root = tree->get_root();
        if (!root) return;

        // Resolved fresh every call: whatever camera is active RIGHT NOW gets the
        // effect, so camera swaps / scene restarts need no script involvement.
        auto* cam = root->get_camera_3d();
        auto* quad = pp_get_quad();   // null if never created OR freed together with its old camera
        if (!cam) {
            if (quad) quad->set_visible(false);
            return;
        }

        if (!pp_material.is_valid()) {
            pp_material.instantiate();
            pp_material->set_shader(shader);
            // The quad reads the screen/depth texture, which Godot only fills with OPAQUE geometry, so it is
            // sorted with the transparent objects and sits right at the near plane = the nearest of them.
            // Transparent lists draw far -> near, so by default it is drawn LAST and paints its opaque-only
            // copy of the screen over every transparent object already drawn (GPU particles, trails, glass...).
            // Lowest render priority makes it draw FIRST instead: it still sees all opaque depth, and everything
            // transparent is then drawn on top of the result. The quad never writes depth (depth_draw_never), so
            // those objects keep depth testing against the real scene.
            pp_material->set_render_priority(godot::Material::RENDER_PRIORITY_MIN);
        }

        if (!quad) {
            godot::Ref<godot::QuadMesh> mesh;
            mesh.instantiate();
            mesh->set_size(godot::Vector2(2.0f, 2.0f));

            quad = memnew(godot::MeshInstance3D);
            quad->set_mesh(mesh);
            quad->set_surface_override_material(0, pp_material);
            quad->set_cast_shadows_setting(godot::GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
            quad->set_gi_mode(godot::GeometryInstance3D::GI_MODE_DISABLED);
            quad->set_visible(false);
            pp_quad_id = godot::ObjectID(quad->get_instance_id());
        }

        // Follow the active camera (also covers a new camera reusing the old address)
        if (quad->get_parent() != cam) {
            if (auto* old_parent = quad->get_parent()) old_parent->remove_child(quad);
            cam->add_child(quad);
        }
        // Updated every frame in case the camera's near clip changes
        quad->set_position(godot::Vector3(0, 0, -(cam->get_near() + 0.001f)));

        // Stamp params from master material into quad's material
        auto list = shader->get_shader_uniform_list();
        for (int i = 0; i < list.size(); i++) {
            auto dict = static_cast<godot::Dictionary>(list[i]);
            godot::StringName name = static_cast<godot::String>(dict.get("name", godot::String()));
            std::string sname = Tool::to_std_string(godot::String(name));
            if (sname == Internal::SENTINEL) continue;
            godot::Variant v = material->get_shader_parameter(name);
            if (v.get_type() != godot::Variant::NIL)
                pp_material->set_shader_parameter(name, v);
        }

        pp_last_frame = godot::Engine::get_singleton()->get_process_frames();
        quad->set_visible(true);
    }

    // Hides postprocess quads that were not drawn this frame. Without this a
    // quad stays visible forever once the script stops calling draw_material
    // (e.g. a toggle key), so the effect could never be turned off.
    void Shader::end_frame_postprocess() {
        if (postprocess_registry.empty()) return;
        uint64_t frame = godot::Engine::get_singleton()->get_process_frames();
        for (auto* pp : postprocess_registry) {
            if (!pp || pp->pp_last_frame == frame) continue;
            auto* quad = pp->pp_get_quad();   // validated; safe if the camera (and quad) were freed
            if (quad && quad->is_visible()) quad->set_visible(false);
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
        return Tool::to_std_string(shader -> get_code());
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
        return build_surface_clone(shader, original);
    }

    // Outputs (emulated MRT) //
    size_t Shader::get_output_count() const {
        return outputs.size();
    }

    std::vector<std::string> Shader::get_output_names() const {
        std::vector<std::string> names;
        for (const auto& output : outputs) names.push_back(output.name);
        return names;
    }

    bool Shader::has_output(const std::string& name) const {
        for (const auto& output : outputs) if (output.name == name) return true;
        return false;
    }

    Rendertarget* Shader::get_output_target(size_t index) const {
        if (index >= outputs.size() || !outputs[index].rendertarget_id.is_valid()) return nullptr;
        auto* object = godot::ObjectDB::get_instance(outputs[index].rendertarget_id);
        return object ? godot::Object::cast_to<Rendertarget>(object) : nullptr;
    }

    godot::Ref<godot::ShaderMaterial> Shader::snapshot_output_material(size_t index) {
        if (index >= outputs.size() || mode != Mode::CanvasItem) return godot::Ref<godot::ShaderMaterial>();
        return outputs[index].pool.next(outputs[index].shader, material);
    }

    godot::Ref<godot::ShaderMaterial> Shader::create_output_surface_material(size_t index, godot::Ref<godot::Material> original) {
        if (index >= outputs.size()) return godot::Ref<godot::ShaderMaterial>();
        return build_surface_clone(outputs[index].shader, original);
    }

    bool Shader::set_output(const std::string& name, Rendertarget* rendertarget) {
        for (auto& output : outputs) {
            if (output.name != name) continue;
            output.rendertarget_id = rendertarget ? godot::ObjectID(rendertarget -> get_instance_id()) : godot::ObjectID();
            return true;
        }
        return false;
    }

    godot::Ref<godot::ShaderMaterial> Shader::build_surface_clone(godot::Ref<godot::Shader> source, godot::Ref<godot::Material> original) {
        godot::Ref<godot::ShaderMaterial> clone;
        clone.instantiate();
        clone -> set_shader(source);
        prune_surface_materials();

        godot::Ref<godot::BaseMaterial3D> base = original;
        auto list = source -> get_shader_uniform_list();
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

    // Setters //
    bool Shader::set_param(const std::string& name, const godot::Variant& value) {
        if (!material.is_valid()) return false;
        if (name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), value);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), value);
        return true;
    }

    bool Shader::set_param_rid(const std::string& name, godot::RID texture) {
        if (!material.is_valid() || name == Internal::SENTINEL || !texture.is_valid()) return false;
        material -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), godot::Variant(texture));
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), godot::Variant(texture));
        return true;
    }

    bool Shader::set_param_texture(const std::string& name, godot::Ref<godot::Texture2D> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), texture);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), texture);
        return true;
    }

    bool Shader::set_param_viewport_texture(const std::string& name, godot::Ref<godot::ViewportTexture> texture) {
        if (!material.is_valid() || name == Internal::SENTINEL) return false;
        material -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), texture);
        prune_surface_materials();
        for (auto& m : surface_materials) m -> set_shader_parameter(godot::StringName(Tool::to_godot_string(name)), texture);
        return true;
    }
}
#endif