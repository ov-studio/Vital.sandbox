/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: gtao.h
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 10/11/2026
     Desc: GTAO Utilities (compositor effect, port of Intel XeGTAO)
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#if defined(VSDK_Client)
#include <Vital.sandbox/Engine/public/core.h>
#include <godot_cpp/classes/compositor.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/rd_shader_source.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>


/////////////////////////
// Vital: Engine: GTAO //
/////////////////////////

namespace Vital::Engine {
    class GTAO : public godot::CompositorEffect {
        GDCLASS(GTAO, godot::CompositorEffect)
        public:
            struct Settings {
                int quality = 2;                            // 0: low, 1: medium, 2: high, 3: ultra
                int denoise = 1;                            // 0: off, 1: sharp, 2: medium, 3: soft
                float radius = 0.5f;
                float intensity = 1.0f;
                float radius_multiplier = 1.457f;
                float falloff_range = 0.615f;
                float sample_distribution_power = 2.0f;
                float thin_occluder_compensation = 0.0f;
                float final_value_power = 2.2f;
                float depth_mip_sampling_offset = 3.30f;
                float blur = 1.2f;                          // denoise center weight: lower = blurrier / smoother
                int half_res = 1;                           // AO computed at half resolution + depth-aware upsample
                int debug = 0;                              // 0: off, 1: show AO, 2: show viewspace depth, 3: solid magenta (color write test)
            };

        private:
            // Mirrors the push constant block of the shaders (112 bytes)
            struct Constants {
                int32_t viewport_size[2];
                float pixel_size[2];
                float depth_unpack[2];
                float tan_half_fov[2];
                float ndc_to_view_mul[2];
                float ndc_to_view_add[2];
                float ndc_to_view_mul_x_pixel_size[2];
                float effect_radius;
                float effect_falloff_range;
                float radius_multiplier;
                float far_cut;
                float final_value_power;
                float denoise_blur_beta;
                float sample_distribution_power;
                float thin_occluder_compensation;
                float depth_mip_sampling_offset;
                int32_t noise_index;
                float intensity;
                int32_t debug_mode;
                int32_t full_size[2];
            };
            static_assert(sizeof(Constants) == 112, "GTAO push constant block must be 112 bytes");

            static std::mutex mutex;
            static Settings settings;
            static bool enabled;
            static std::atomic<uint32_t> frame_index;

            // Render thread state
            bool ready = false;
            bool failed = false;
            bool logged = false;
            bool info_logged = false;
            bool warned = false;
            godot::RID sampler;
            godot::RID prefilter_shader, prefilter_pipeline;
            godot::RID main_shader[4], main_pipeline[4];
            godot::RID denoise_shader, denoise_pipeline;
            godot::RID final_shader, final_pipeline;
            godot::RID upsample_shader, upsample_pipeline;

            bool compile(godot::RenderingDevice* rd, const std::string& source, const char* name, godot::RID& shader, godot::RID& pipeline);
            bool init_resources(godot::RenderingDevice* rd);
            bool init_main(godot::RenderingDevice* rd, int quality);
            void free_resources();

        protected:
            static void _bind_methods() {}

        public:
            // Hooks //
            void _notification(int what);
            void _render_callback(int32_t type, godot::RenderData* data) override;

            // Managers //
            static Settings get_settings();
            static void update_settings(const std::function<void(Settings&)>& exec);
            static bool is_enabled();
            static void set_enabled(bool state);
            static void reset();
    };
}
#endif
