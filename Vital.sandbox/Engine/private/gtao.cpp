/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Engine: gtao.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 10/11/2026
     Desc: GTAO Utilities (compositor effect, port of Intel XeGTAO)
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#include <Vital.sandbox/Engine/public/gtao.h>
#include <godot_cpp/classes/rd_texture_format.hpp>
#if defined(VSDK_Client)


/////////////////////////
// Vital: Engine: GTAO //
/////////////////////////

// Shaders: Intel XeGTAO (MIT) ported to GLSL compute. Passes:
//   prefilter -> viewspace depth MIP chain | main -> AO term + edges | denoise -> edge-aware blur (last pass applies AO onto the scene color)
namespace {
    constexpr const char* shader_common = R"GLSL(#version 450

// Port of Intel XeGTAO (MIT) - https://github.com/GameTechDev/XeGTAO
layout(push_constant, std430) uniform Params {
    ivec2 viewport_size;
    vec2 pixel_size;
    vec2 depth_unpack;
    vec2 tan_half_fov;
    vec2 ndc_to_view_mul;
    vec2 ndc_to_view_add;
    vec2 ndc_to_view_mul_x_pixel_size;
    float effect_radius;
    float effect_falloff_range;
    float radius_multiplier;
    float far_cut;
    float final_value_power;
    float denoise_blur_beta;
    float sample_distribution_power;
    float thin_occluder_compensation;
    float depth_mip_sampling_offset;
    int noise_index;
    float intensity;
    int debug_mode;
    ivec2 full_size;
} p;

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398
#define DEPTH_MIP_LEVELS 5
#define OCCLUSION_TERM_SCALE 1.5

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

vec4 unpack_edges(float v) {
    uint pv = uint(v * 255.5);
    vec4 e;
    e.x = float((pv >> 6) & 3u) / 3.0;
    e.y = float((pv >> 4) & 3u) / 3.0;
    e.z = float((pv >> 2) & 3u) / 3.0;
    e.w = float((pv >> 0) & 3u) / 3.0;
    return clamp(e, 0.0, 1.0);
}
)GLSL";
    constexpr const char* shader_prefilter = R"GLSL(layout(set = 0, binding = 0) uniform sampler2D src_depth;
layout(r32f, set = 0, binding = 1) uniform restrict writeonly image2D out_depth0;
layout(r32f, set = 0, binding = 2) uniform restrict writeonly image2D out_depth1;
layout(r32f, set = 0, binding = 3) uniform restrict writeonly image2D out_depth2;
layout(r32f, set = 0, binding = 4) uniform restrict writeonly image2D out_depth3;
layout(r32f, set = 0, binding = 5) uniform restrict writeonly image2D out_depth4;

shared float scratch[8][8];

float linearize(float d) {
    float den = min(p.depth_unpack.y - d, -1e-20);
    return clamp(p.depth_unpack.x / den, 0.0, 3.402823466e+38);
}

float mip_filter(float d0, float d1, float d2, float d3) {
    float max_depth = max(max(d0, d1), max(d2, d3));
    const float depth_range_scale_factor = 0.75;
    float effect_radius = depth_range_scale_factor * p.effect_radius * p.radius_multiplier;
    float falloff_range = p.effect_falloff_range * effect_radius;
    float falloff_from = effect_radius * (1.0 - p.effect_falloff_range);
    float falloff_mul = -1.0 / falloff_range;
    float falloff_add = falloff_from / falloff_range + 1.0;
    float w0 = clamp((max_depth - d0) * falloff_mul + falloff_add, 0.0, 1.0);
    float w1 = clamp((max_depth - d1) * falloff_mul + falloff_add, 0.0, 1.0);
    float w2 = clamp((max_depth - d2) * falloff_mul + falloff_add, 0.0, 1.0);
    float w3 = clamp((max_depth - d3) * falloff_mul + falloff_add, 0.0, 1.0);
    float sum = w0 + w1 + w2 + w3;
    return (w0 * d0 + w1 * d1 + w2 * d2 + w3 * d3) / sum;
}

bool inside(ivec2 c, ivec2 size) {
    return c.x < size.x && c.y < size.y;
}

void main() {
    ivec2 base = ivec2(gl_GlobalInvocationID.xy);
    ivec2 gt = ivec2(gl_LocalInvocationID.xy);
    ivec2 size = p.full_size;
    ivec2 mx = size - 1;
    ivec2 pix = base * 2;

    // MIP 0 (barriers below require every invocation to run, so reads are clamped and only writes are guarded)
    float d0 = linearize(texelFetch(src_depth, min(pix, mx), 0).r);
    float d1 = linearize(texelFetch(src_depth, min(pix + ivec2(1, 0), mx), 0).r);
    float d2 = linearize(texelFetch(src_depth, min(pix + ivec2(0, 1), mx), 0).r);
    float d3 = linearize(texelFetch(src_depth, min(pix + ivec2(1, 1), mx), 0).r);
    if (inside(pix, size)) imageStore(out_depth0, pix, vec4(d0));
    if (inside(pix + ivec2(1, 0), size)) imageStore(out_depth0, pix + ivec2(1, 0), vec4(d1));
    if (inside(pix + ivec2(0, 1), size)) imageStore(out_depth0, pix + ivec2(0, 1), vec4(d2));
    if (inside(pix + ivec2(1, 1), size)) imageStore(out_depth0, pix + ivec2(1, 1), vec4(d3));

    // MIP 1
    float dm1 = mip_filter(d0, d1, d2, d3);
    if (inside(base, max(ivec2(1), size >> 1))) imageStore(out_depth1, base, vec4(dm1));
    scratch[gt.x][gt.y] = dm1;
    memoryBarrierShared();
    barrier();

    // MIP 2
    if (gt.x % 2 == 0 && gt.y % 2 == 0) {
        float tl = scratch[gt.x + 0][gt.y + 0];
        float tr = scratch[gt.x + 1][gt.y + 0];
        float bl = scratch[gt.x + 0][gt.y + 1];
        float br = scratch[gt.x + 1][gt.y + 1];
        float dm2 = mip_filter(tl, tr, bl, br);
        if (inside(base / 2, max(ivec2(1), size >> 2))) imageStore(out_depth2, base / 2, vec4(dm2));
        scratch[gt.x][gt.y] = dm2;
    }
    memoryBarrierShared();
    barrier();

    // MIP 3
    if (gt.x % 4 == 0 && gt.y % 4 == 0) {
        float tl = scratch[gt.x + 0][gt.y + 0];
        float tr = scratch[gt.x + 2][gt.y + 0];
        float bl = scratch[gt.x + 0][gt.y + 2];
        float br = scratch[gt.x + 2][gt.y + 2];
        float dm3 = mip_filter(tl, tr, bl, br);
        if (inside(base / 4, max(ivec2(1), size >> 3))) imageStore(out_depth3, base / 4, vec4(dm3));
        scratch[gt.x][gt.y] = dm3;
    }
    memoryBarrierShared();
    barrier();

    // MIP 4
    if (gt.x % 8 == 0 && gt.y % 8 == 0) {
        float tl = scratch[gt.x + 0][gt.y + 0];
        float tr = scratch[gt.x + 4][gt.y + 0];
        float bl = scratch[gt.x + 0][gt.y + 4];
        float br = scratch[gt.x + 4][gt.y + 4];
        float dm4 = mip_filter(tl, tr, bl, br);
        if (inside(base / 8, max(ivec2(1), size >> 4))) imageStore(out_depth4, base / 8, vec4(dm4));
    }
}
)GLSL";
    constexpr const char* shader_main = R"GLSL(layout(set = 0, binding = 0) uniform sampler2D depth_mips;
layout(r8, set = 0, binding = 1) uniform restrict writeonly image2D out_ao;
layout(r8, set = 0, binding = 2) uniform restrict writeonly image2D out_edges;

vec3 view_position(vec2 screen_pos, float view_z) {
    return vec3((p.ndc_to_view_mul * screen_pos + p.ndc_to_view_add) * view_z, view_z);
}

vec4 calc_edges(float c, float l, float r, float t, float b) {
    vec4 e = vec4(l, r, t, b) - c;
    float slope_lr = (e.y - e.x) * 0.5;
    float slope_tb = (e.w - e.z) * 0.5;
    vec4 adjusted = e + vec4(slope_lr, -slope_lr, slope_tb, -slope_tb);
    e = min(abs(e), abs(adjusted));
    return clamp(1.25 - e / (c * 0.011), 0.0, 1.0);
}

float pack_edges(vec4 e) {
    e = roundEven(clamp(e, 0.0, 1.0) * 2.9);
    return dot(e, vec4(64.0 / 255.0, 16.0 / 255.0, 4.0 / 255.0, 1.0 / 255.0));
}

vec3 calc_normal(vec4 edges, vec3 c, vec3 l, vec3 r, vec3 t, vec3 b) {
    vec4 accepted = clamp(vec4(edges.x * edges.z, edges.z * edges.y, edges.y * edges.w, edges.w * edges.x) + 0.01, 0.0, 1.0);
    l = normalize(l - c);
    r = normalize(r - c);
    t = normalize(t - c);
    b = normalize(b - c);
    vec3 n = accepted.x * cross(l, t) + accepted.y * cross(t, r) + accepted.z * cross(r, b) + accepted.w * cross(b, l);
    return normalize(n);
}

float fast_sqrt(float x) {
    return uintBitsToFloat(0x1fbd1df5u + (floatBitsToUint(x) >> 1));
}

float fast_acos(float v) {
    float x = abs(v);
    float res = -0.156583 * x + PI_HALF;
    res *= fast_sqrt(max(1.0 - x, 0.0));
    return (v >= 0.0) ? res : PI - res;
}

uint hilbert_index(uint x, uint y) {
    x &= 63u;
    y &= 63u;
    uint index = 0u;
    for (uint level = 32u; level > 0u; level /= 2u) {
        uint rx = ((x & level) > 0u) ? 1u : 0u;
        uint ry = ((y & level) > 0u) ? 1u : 0u;
        index += level * level * ((3u * rx) ^ ry);
        if (ry == 0u) {
            if (rx == 1u) {
                x = 63u - x;
                y = 63u - y;
            }
            uint tmp = x;
            x = y;
            y = tmp;
        }
    }
    return index;
}

vec2 spatio_temporal_noise(ivec2 pix, uint temporal_index) {
    uint index = hilbert_index(uint(pix.x), uint(pix.y));
    index += 288u * (temporal_index % 64u);
    return fract(0.5 + float(index) * vec2(0.75487766624669276005, 0.5698402909980532659114));
}

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(pix, p.viewport_size))) return;
    int base_mip = (p.full_size.x != p.viewport_size.x || p.full_size.y != p.viewport_size.y) ? 1 : 0;
    ivec2 mx = p.viewport_size - 1;
    vec2 ps = p.pixel_size;
    vec2 uv = (vec2(pix) + 0.5) * ps;

    float view_z = texelFetch(depth_mips, pix, base_mip).r;
    float pix_lz = texelFetch(depth_mips, clamp(pix + ivec2(-1, 0), ivec2(0), mx), base_mip).r;
    float pix_rz = texelFetch(depth_mips, clamp(pix + ivec2(1, 0), ivec2(0), mx), base_mip).r;
    float pix_tz = texelFetch(depth_mips, clamp(pix + ivec2(0, -1), ivec2(0), mx), base_mip).r;
    float pix_bz = texelFetch(depth_mips, clamp(pix + ivec2(0, 1), ivec2(0), mx), base_mip).r;

    vec4 edges = calc_edges(view_z, pix_lz, pix_rz, pix_tz, pix_bz);
    imageStore(out_edges, pix, vec4(pack_edges(edges)));

    // Debug 2: viewspace depth (0..75m) instead of AO; sky stays white
    if (p.debug_mode == 2) {
        imageStore(out_ao, pix, vec4(clamp(view_z / 75.0, 0.0, 1.0) / OCCLUSION_TERM_SCALE));
        return;
    }

    // Sky / far plane: nothing to occlude, store full visibility
    if (view_z >= p.far_cut) {
        imageStore(out_ao, pix, vec4(1.0 / OCCLUSION_TERM_SCALE));
        return;
    }

    // Viewspace normal generated in place from depth
    vec3 center = view_position(uv, view_z);
    vec3 left = view_position(uv + vec2(-1.0, 0.0) * ps, pix_lz);
    vec3 right = view_position(uv + vec2(1.0, 0.0) * ps, pix_rz);
    vec3 top = view_position(uv + vec2(0.0, -1.0) * ps, pix_tz);
    vec3 bottom = view_position(uv + vec2(0.0, 1.0) * ps, pix_bz);
    vec3 view_normal = calc_normal(edges, center, left, right, top, bottom);

    // Move center pixel slightly towards camera to avoid imprecision artifacts (FP32 depth)
    view_z *= 0.99999;

    vec3 pix_center_pos = view_position(uv, view_z);
    vec3 view_vec = normalize(-pix_center_pos);

    float effect_radius = p.effect_radius * p.radius_multiplier;
    float falloff_range = p.effect_falloff_range * effect_radius;
    float falloff_from = effect_radius * (1.0 - p.effect_falloff_range);
    float falloff_mul = -1.0 / falloff_range;
    float falloff_add = falloff_from / falloff_range + 1.0;

    float visibility = 0.0;
    vec2 local_noise = spatio_temporal_noise(pix, uint(p.noise_index));
    float noise_slice = local_noise.x;
    float noise_sample = local_noise.y;

    const float pixel_too_close_threshold = 1.3;
    vec2 pixel_size_at_center_z = vec2(view_z) * p.ndc_to_view_mul_x_pixel_size;
    float screenspace_radius = effect_radius / pixel_size_at_center_z.x;

    // fade out for small screen radii
    visibility += clamp((10.0 - screenspace_radius) / 100.0, 0.0, 1.0) * 0.5;

    // min distance to start sampling from to avoid sampling the center pixel
    float min_s = pixel_too_close_threshold / screenspace_radius;

    for (int slice_i = 0; slice_i < SLICE_COUNT; slice_i++) {
        float slice_f = float(slice_i);
        float slice_k = (slice_f + noise_slice) / float(SLICE_COUNT);
        float phi = slice_k * PI;
        float cos_phi = cos(phi);
        float sin_phi = sin(phi);
        vec2 omega = vec2(cos_phi, -sin_phi) * screenspace_radius;

        vec3 direction_vec = vec3(cos_phi, sin_phi, 0.0);
        vec3 ortho_direction_vec = direction_vec - (dot(direction_vec, view_vec) * view_vec);
        vec3 axis_vec = normalize(cross(ortho_direction_vec, view_vec));
        vec3 projected_normal_vec = view_normal - axis_vec * dot(view_normal, axis_vec);
        float sign_norm = sign(dot(ortho_direction_vec, projected_normal_vec));
        float projected_normal_vec_length = length(projected_normal_vec);
        float cos_norm = clamp(dot(projected_normal_vec, view_vec) / projected_normal_vec_length, 0.0, 1.0);
        float n = sign_norm * fast_acos(cos_norm);

        float low_horizon_cos0 = cos(n + PI_HALF);
        float low_horizon_cos1 = cos(n - PI_HALF);
        float horizon_cos0 = low_horizon_cos0;
        float horizon_cos1 = low_horizon_cos1;

        for (int step_i = 0; step_i < STEPS_PER_SLICE; step_i++) {
            float step_f = float(step_i);
            // R1 sequence (http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/)
            float step_base_noise = (slice_f + step_f * float(STEPS_PER_SLICE)) * 0.6180339887498948482;
            float step_noise = fract(noise_sample + step_base_noise);

            float s = (step_f + step_noise) / float(STEPS_PER_SLICE);
            s = pow(s, p.sample_distribution_power);
            s += min_s;

            vec2 sample_offset = s * omega;
            float sample_offset_length = length(sample_offset);
            float mip_level = clamp(log2(sample_offset_length) - p.depth_mip_sampling_offset, 0.0, float(DEPTH_MIP_LEVELS));

            // snap to pixel center
            sample_offset = roundEven(sample_offset) * ps;

            vec2 sample_screen_pos0 = uv + sample_offset;
            float sz0 = textureLod(depth_mips, sample_screen_pos0, mip_level + float(base_mip)).r;
            vec3 sample_pos0 = view_position(sample_screen_pos0, sz0);

            vec2 sample_screen_pos1 = uv - sample_offset;
            float sz1 = textureLod(depth_mips, sample_screen_pos1, mip_level + float(base_mip)).r;
            vec3 sample_pos1 = view_position(sample_screen_pos1, sz1);

            vec3 sample_delta0 = sample_pos0 - pix_center_pos;
            vec3 sample_delta1 = sample_pos1 - pix_center_pos;
            float sample_dist0 = length(sample_delta0);
            float sample_dist1 = length(sample_delta1);

            vec3 sample_horizon_vec0 = sample_delta0 / sample_dist0;
            vec3 sample_horizon_vec1 = sample_delta1 / sample_dist1;

            // thickness heuristic + falloff
            float falloff_base0 = length(vec3(sample_delta0.x, sample_delta0.y, sample_delta0.z * (1.0 + p.thin_occluder_compensation)));
            float falloff_base1 = length(vec3(sample_delta1.x, sample_delta1.y, sample_delta1.z * (1.0 + p.thin_occluder_compensation)));
            float weight0 = clamp(falloff_base0 * falloff_mul + falloff_add, 0.0, 1.0);
            float weight1 = clamp(falloff_base1 * falloff_mul + falloff_add, 0.0, 1.0);

            float shc0 = dot(sample_horizon_vec0, view_vec);
            float shc1 = dot(sample_horizon_vec1, view_vec);

            // discard unwanted samples
            shc0 = mix(low_horizon_cos0, shc0, weight0);
            shc1 = mix(low_horizon_cos1, shc1, weight1);

            horizon_cos0 = max(horizon_cos0, shc0);
            horizon_cos1 = max(horizon_cos1, shc1);
        }

        // fudge for slight overdarkening on high slopes (from XeGTAO)
        projected_normal_vec_length = mix(projected_normal_vec_length, 1.0, 0.05);

        float h0 = -fast_acos(horizon_cos1);
        float h1 = fast_acos(horizon_cos0);
        float iarc0 = (cos_norm + 2.0 * h0 * sin(n) - cos(2.0 * h0 - n)) / 4.0;
        float iarc1 = (cos_norm + 2.0 * h1 * sin(n) - cos(2.0 * h1 - n)) / 4.0;
        float local_visibility = projected_normal_vec_length * (iarc0 + iarc1);
        visibility += local_visibility;
    }

    visibility /= float(SLICE_COUNT);
    visibility = pow(max(visibility, 0.0), p.final_value_power);
    visibility = max(0.03, visibility); // disallow total occlusion

    imageStore(out_ao, pix, vec4(clamp(visibility / OCCLUSION_TERM_SCALE, 0.0, 1.0)));
}
)GLSL";
    constexpr const char* shader_denoise = R"GLSL(layout(set = 0, binding = 0) uniform sampler2D src_ao;
layout(set = 0, binding = 1) uniform sampler2D src_edges;
#ifdef FINAL_PASS
layout(rgba16f, set = 0, binding = 2) uniform restrict image2D color_image;
#else
layout(r8, set = 0, binding = 2) uniform restrict writeonly image2D out_ao;
#endif

void main() {
    // 2 horizontal pixels per invocation
    ivec2 base = ivec2(gl_GlobalInvocationID.xy) * ivec2(2, 1);
    if (base.x >= p.viewport_size.x || base.y >= p.viewport_size.y) return;

    #ifdef FINAL_PASS
    float blur_amount = p.denoise_blur_beta;
    #else
    float blur_amount = p.denoise_blur_beta / 5.0;
    #endif
    const float diag_weight = 0.85 * 0.5;
    vec2 ps = p.pixel_size;

    // gather edge and visibility quads
    vec2 gather_center = vec2(base) * ps;
    vec4 edges_q0 = textureGather(src_edges, gather_center + vec2(0.0, 0.0) * ps, 0);
    vec4 edges_q1 = textureGather(src_edges, gather_center + vec2(2.0, 0.0) * ps, 0);
    vec4 edges_q2 = textureGather(src_edges, gather_center + vec2(1.0, 2.0) * ps, 0);

    vec4 vis_q0 = textureGather(src_ao, gather_center + vec2(0.0, 0.0) * ps, 0);
    vec4 vis_q1 = textureGather(src_ao, gather_center + vec2(2.0, 0.0) * ps, 0);
    vec4 vis_q2 = textureGather(src_ao, gather_center + vec2(0.0, 2.0) * ps, 0);
    vec4 vis_q3 = textureGather(src_ao, gather_center + vec2(2.0, 2.0) * ps, 0);

    for (int side = 0; side < 2; side++) {
        ivec2 pix = ivec2(base.x + side, base.y);
        bool s0 = (side == 0);

        vec4 edges_l = unpack_edges(s0 ? edges_q0.x : edges_q0.y);
        vec4 edges_t = unpack_edges(s0 ? edges_q0.z : edges_q1.w);
        vec4 edges_r = unpack_edges(s0 ? edges_q1.x : edges_q1.y);
        vec4 edges_b = unpack_edges(s0 ? edges_q2.w : edges_q2.z);
        vec4 edges_c = unpack_edges(s0 ? edges_q0.y : edges_q1.x);

        // enforce edge symmetry for a slightly sharper blur
        edges_c *= vec4(edges_l.y, edges_r.x, edges_t.w, edges_b.z);

        // allow small amount of AO leaking from neighbours if there are 3 or 4 edges (reduces aliasing)
        const float leak_threshold = 2.5;
        const float leak_strength = 0.5;
        float edginess = (clamp(4.0 - leak_threshold - dot(edges_c, vec4(1.0)), 0.0, 1.0) / (4.0 - leak_threshold)) * leak_strength;
        edges_c = clamp(edges_c + edginess, 0.0, 1.0);

        // diagonals
        float weight_tl = diag_weight * (edges_c.x * edges_l.z + edges_c.z * edges_t.x);
        float weight_tr = diag_weight * (edges_c.z * edges_t.y + edges_c.y * edges_r.z);
        float weight_bl = diag_weight * (edges_c.w * edges_b.x + edges_c.x * edges_l.w);
        float weight_br = diag_weight * (edges_c.y * edges_r.w + edges_c.w * edges_b.y);

        float ao_c = s0 ? vis_q0.y : vis_q1.x;
        float ao_l = s0 ? vis_q0.x : vis_q0.y;
        float ao_t = s0 ? vis_q0.z : vis_q1.w;
        float ao_r = s0 ? vis_q1.x : vis_q1.y;
        float ao_b = s0 ? vis_q2.z : vis_q3.w;
        float ao_tl = s0 ? vis_q0.w : vis_q0.z;
        float ao_br = s0 ? vis_q3.w : vis_q3.z;
        float ao_tr = s0 ? vis_q1.w : vis_q1.z;
        float ao_bl = s0 ? vis_q2.w : vis_q2.z;

        float sum_weight = blur_amount;
        float sum = ao_c * sum_weight;

        sum += edges_c.x * ao_l;  sum_weight += edges_c.x;
        sum += edges_c.y * ao_r;  sum_weight += edges_c.y;
        sum += edges_c.z * ao_t;  sum_weight += edges_c.z;
        sum += edges_c.w * ao_b;  sum_weight += edges_c.w;

        sum += weight_tl * ao_tl; sum_weight += weight_tl;
        sum += weight_tr * ao_tr; sum_weight += weight_tr;
        sum += weight_bl * ao_bl; sum_weight += weight_bl;
        sum += weight_br * ao_br; sum_weight += weight_br;

        float value = sum / sum_weight;

        if (pix.x >= p.viewport_size.x) continue;

        #ifdef FINAL_PASS
        float visibility = clamp(value * OCCLUSION_TERM_SCALE, 0.0, 1.0);
        float factor = mix(1.0, visibility, p.intensity);
        if (p.debug_mode == 3) {
            imageStore(color_image, pix, vec4(1.0, 0.0, 1.0, 1.0));
        }
        else if (p.debug_mode != 0) {
            imageStore(color_image, pix, vec4(vec3(visibility), 1.0));
        }
        else if (factor < 0.999) {
            vec4 color = imageLoad(color_image, pix);
            color.rgb *= factor;
            imageStore(color_image, pix, color);
        }
        #else
        imageStore(out_ao, pix, vec4(value));
        #endif
    }
}
)GLSL";
    constexpr const char* shader_upsample = R"GLSL(layout(set = 0, binding = 0) uniform sampler2D src_ao;
layout(set = 0, binding = 1) uniform sampler2D depth_mips;
layout(rgba16f, set = 0, binding = 2) uniform restrict image2D color_image;

// Depth-aware upsample of the half resolution AO, applied straight onto the scene color
void main() {
    ivec2 f = ivec2(gl_GlobalInvocationID.xy);
    if (any(greaterThanEqual(f, p.full_size))) return;

    float depth_full = texelFetch(depth_mips, f, 0).r;
    if (p.debug_mode == 0 && depth_full >= p.far_cut) return;

    // Depth tolerance follows the local depth slope, otherwise grazing surfaces (floors) reject nearly every neighbour
    ivec2 fm = p.full_size - 1;
    float dl = texelFetch(depth_mips, clamp(f + ivec2(-1, 0), ivec2(0), fm), 0).r;
    float dr = texelFetch(depth_mips, clamp(f + ivec2(1, 0), ivec2(0), fm), 0).r;
    float dt = texelFetch(depth_mips, clamp(f + ivec2(0, -1), ivec2(0), fm), 0).r;
    float db = texelFetch(depth_mips, clamp(f + ivec2(0, 1), ivec2(0), fm), 0).r;
    float tolerance = 0.01 * depth_full + 1.5 * max(abs(dr - dl), abs(db - dt)) + 1e-4;

    ivec2 ao_size = p.viewport_size;
    vec2 pos = (vec2(f) + 0.5) * vec2(ao_size) / vec2(p.full_size) - 0.5;
    ivec2 i0 = ivec2(floor(pos));
    vec2 fr = pos - vec2(i0);

    float sum = 0.0;
    float sum_weight = 0.0;
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 2; i++) {
            ivec2 t = clamp(i0 + ivec2(i, j), ivec2(0), ao_size - 1);
            float bilinear = ((i == 0) ? 1.0 - fr.x : fr.x) * ((j == 0) ? 1.0 - fr.y : fr.y);
            float depth_half = texelFetch(depth_mips, t, 1).r;
            float depth_weight = clamp(1.0 - abs(depth_half - depth_full) / tolerance, 0.0, 1.0);
            float w = bilinear * (depth_weight + 1e-3);
            sum += w * texelFetch(src_ao, t, 0).r;
            sum_weight += w;
        }
    }

    float visibility = clamp((sum / sum_weight) * OCCLUSION_TERM_SCALE, 0.0, 1.0);
    float factor = mix(1.0, visibility, p.intensity);
    if (p.debug_mode == 3) {
        imageStore(color_image, f, vec4(1.0, 0.0, 1.0, 1.0));
    }
    else if (p.debug_mode != 0) {
        imageStore(color_image, f, vec4(vec3(visibility), 1.0));
    }
    else if (factor < 0.999) {
        vec4 color = imageLoad(color_image, f);
        color.rgb *= factor;
        imageStore(color_image, f, color);
    }
}
)GLSL";

    // Godot's own console is not visible in the sandbox, so mirror messages there (marshalled to the main thread)
    void report(const std::string& message, bool error = false) {
        if (error) godot::UtilityFunctions::push_error(godot::String::utf8(message.c_str()));
        auto core = Vital::Engine::Core::get_singleton();
        if (core) core -> enqueue([message]() { Vital::Tool::print("sbox", message); });
    }
}

namespace Vital::Engine {
    std::mutex GTAO::mutex;
    GTAO::Settings GTAO::settings;
    bool GTAO::enabled = false;
    std::atomic<uint32_t> GTAO::frame_index { 0 };

    // Resources //
    bool GTAO::compile(godot::RenderingDevice* rd, const std::string& source, const char* name, godot::RID& shader, godot::RID& pipeline) {
        godot::Ref<godot::RDShaderSource> src;
        src.instantiate();
        src -> set_language(godot::RenderingDevice::SHADER_LANGUAGE_GLSL);
        src -> set_stage_source(godot::RenderingDevice::SHADER_STAGE_COMPUTE, godot::String::utf8(source.c_str()));
        godot::Ref<godot::RDShaderSPIRV> spirv = rd -> shader_compile_spirv_from_source(src, true);
        auto error = spirv -> get_stage_compile_error(godot::RenderingDevice::SHADER_STAGE_COMPUTE);
        if (!error.is_empty()) {
            report(std::string("[GTAO] failed to compile '") + name + "': " + std::string(error.utf8().get_data()), true);
            return false;
        }
        shader = rd -> shader_create_from_spirv(spirv, name);
        if (!shader.is_valid()) return false;
        pipeline = rd -> compute_pipeline_create(shader);
        return pipeline.is_valid();
    }

    bool GTAO::init_resources(godot::RenderingDevice* rd) {
        godot::Ref<godot::RDSamplerState> state;
        state.instantiate();
        state -> set_min_filter(godot::RenderingDevice::SAMPLER_FILTER_NEAREST);
        state -> set_mag_filter(godot::RenderingDevice::SAMPLER_FILTER_NEAREST);
        state -> set_mip_filter(godot::RenderingDevice::SAMPLER_FILTER_NEAREST);
        state -> set_repeat_u(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        state -> set_repeat_v(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        state -> set_repeat_w(godot::RenderingDevice::SAMPLER_REPEAT_MODE_CLAMP_TO_EDGE);
        sampler = rd -> sampler_create(state);
        std::string common = shader_common;
        return sampler.is_valid()
            && compile(rd, common + shader_prefilter, "gtao_prefilter", prefilter_shader, prefilter_pipeline)
            && compile(rd, common + shader_denoise, "gtao_denoise", denoise_shader, denoise_pipeline)
            && compile(rd, common + "#define FINAL_PASS\n" + shader_denoise, "gtao_final", final_shader, final_pipeline)
            && compile(rd, common + shader_upsample, "gtao_upsample", upsample_shader, upsample_pipeline);
    }

    bool GTAO::init_main(godot::RenderingDevice* rd, int quality) {
        if (main_pipeline[quality].is_valid()) return true;
        static constexpr int slices[4] = { 1, 2, 3, 9 };
        static constexpr int steps[4] = { 2, 2, 3, 3 };
        std::string source = std::string(shader_common)
            + "#define SLICE_COUNT " + std::to_string(slices[quality]) + "\n"
            + "#define STEPS_PER_SLICE " + std::to_string(steps[quality]) + "\n"
            + shader_main;
        return compile(rd, source, "gtao_main", main_shader[quality], main_pipeline[quality]);
    }

    void GTAO::free_resources() {
        auto rs = godot::RenderingServer::get_singleton();
        auto rd = rs ? rs -> get_rendering_device() : nullptr;
        if (!rd) return;
        // Pipelines are released together with their shader
        auto release = [&](godot::RID& rid) {
            if (rid.is_valid()) rd -> free_rid(rid);
            rid = godot::RID();
        };
        release(prefilter_shader);
        release(denoise_shader);
        release(final_shader);
        release(upsample_shader);
        for (auto& shader : main_shader) release(shader);
        release(sampler);
        prefilter_pipeline = denoise_pipeline = final_pipeline = upsample_pipeline = godot::RID();
        for (auto& pipeline : main_pipeline) pipeline = godot::RID();
        ready = false;
    }

    // Hooks //
    void GTAO::_notification(int what) {
        if (what == NOTIFICATION_PREDELETE) free_resources();
    }

    void GTAO::_render_callback(int32_t type, godot::RenderData* data) {
        if (!data || failed) return;
        auto rs = godot::RenderingServer::get_singleton();
        auto rd = rs ? rs -> get_rendering_device() : nullptr;
        godot::Ref<godot::RenderSceneBuffers> base_buffers = data -> get_render_scene_buffers();
        auto scene = data -> get_render_scene_data();
        auto buffers = godot::Object::cast_to<godot::RenderSceneBuffersRD>(base_buffers.ptr());
        if (!rd || !buffers || !scene) return;
        if (!ready) {
            ready = init_resources(rd);
            if (!ready) { failed = true; return; }
        }

        godot::StringName context = "vital_gtao";   // never a global: StringName must not be constructed before the extension initializes
        auto config = get_settings();
        if (!init_main(rd, config.quality)) { failed = true; return; }
        auto size = buffers -> get_internal_size();
        if (size.x < 1 || size.y < 1) return;
        auto frame = frame_index.fetch_add(1);
        bool use_noise = config.denoise > 0 && buffers -> get_use_taa();   // temporal noise only resolves with TAA
        int passes = std::max(config.denoise, 1);
        bool half = config.half_res != 0;
        godot::Vector2i ao_size = half ? godot::Vector2i(std::max(1, size.x >> 1), std::max(1, size.y >> 1)) : size;

        auto uniform = [](godot::RenderingDevice::UniformType type, int binding, godot::RID a, godot::RID b = godot::RID()) {
            godot::Ref<godot::RDUniform> u;
            u.instantiate();
            u -> set_uniform_type(type);
            u -> set_binding(binding);
            u -> add_id(a);
            if (b.is_valid()) u -> add_id(b);
            return u;
        };
        auto image = [&](int binding, godot::RID texture) { return uniform(godot::RenderingDevice::UNIFORM_TYPE_IMAGE, binding, texture); };
        auto sampled = [&](int binding, godot::RID texture) { return uniform(godot::RenderingDevice::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, binding, sampler, texture); };
        auto ensure = [&](const godot::StringName& name, godot::RenderingDevice::DataFormat format, int mips, godot::Vector2i dim) {
            if (buffers -> has_texture(context, name)) return;
            uint32_t usage = godot::RenderingDevice::TEXTURE_USAGE_SAMPLING_BIT | godot::RenderingDevice::TEXTURE_USAGE_STORAGE_BIT;
            buffers -> create_texture(context, name, format, usage, godot::RenderingDevice::TEXTURE_SAMPLES_1, dim, 1, mips, true, false);
        };

        int views = buffers -> get_view_count();
        for (int view = 0; view < views; view++) {
            godot::Projection proj = scene -> get_view_projection(view);
            if (proj.is_orthogonal()) continue;

            godot::String suffix = "_" + godot::String::num_int64(view);
            godot::StringName name_depth = godot::String("depth_mips") + suffix;
            godot::String res_tag = half ? "_h" : "_f";   // AO textures are sized per mode
            godot::StringName name_ao_a = godot::String("ao_a") + suffix + res_tag;
            godot::StringName name_ao_b = godot::String("ao_b") + suffix + res_tag;
            godot::StringName name_edges = godot::String("edges") + suffix + res_tag;
            bool need_b = half || passes > 1;
            ensure(name_depth, godot::RenderingDevice::DATA_FORMAT_R32_SFLOAT, 5, size);
            ensure(name_ao_a, godot::RenderingDevice::DATA_FORMAT_R8_UNORM, 1, ao_size);
            ensure(name_edges, godot::RenderingDevice::DATA_FORMAT_R8_UNORM, 1, ao_size);
            if (need_b) ensure(name_ao_b, godot::RenderingDevice::DATA_FORMAT_R8_UNORM, 1, ao_size);

            // The view projection already carries Godot's Vulkan depth correction (reverse-Z, y-flip):
            // depth = Q32 / z - Q22  =>  z = mul / (add - depth) with mul = -Q32, add = -Q22
            float mul = -(float)proj.columns[3][2];
            float add = -(float)proj.columns[2][2];
            float tan_x = std::abs(1.0f / (float)proj.columns[0][0]);
            float tan_y = std::abs(1.0f / (float)proj.columns[1][1]);
            Constants c {};
            c.viewport_size[0] = ao_size.x;
            c.viewport_size[1] = ao_size.y;
            c.full_size[0] = size.x;
            c.full_size[1] = size.y;
            c.pixel_size[0] = 1.0f / ao_size.x;
            c.pixel_size[1] = 1.0f / ao_size.y;
            c.depth_unpack[0] = mul;
            c.depth_unpack[1] = add;
            c.tan_half_fov[0] = tan_x;
            c.tan_half_fov[1] = tan_y;
            c.ndc_to_view_mul[0] = tan_x * 2.0f;
            c.ndc_to_view_mul[1] = tan_y * -2.0f;
            c.ndc_to_view_add[0] = -tan_x;
            c.ndc_to_view_add[1] = tan_y;
            c.ndc_to_view_mul_x_pixel_size[0] = c.ndc_to_view_mul[0] * c.pixel_size[0];
            c.ndc_to_view_mul_x_pixel_size[1] = c.ndc_to_view_mul[1] * c.pixel_size[1];
            c.effect_radius = config.radius;
            c.effect_falloff_range = config.falloff_range;
            c.radius_multiplier = config.radius_multiplier;
            c.far_cut = (add < -1e-12f) ? 0.999f * (mul / add) : 1.0e18f;
            c.final_value_power = config.final_value_power;
            c.denoise_blur_beta = config.denoise == 0 ? 1e4f : config.blur;
            c.sample_distribution_power = config.sample_distribution_power;
            c.thin_occluder_compensation = config.thin_occluder_compensation;
            c.depth_mip_sampling_offset = config.depth_mip_sampling_offset;
            c.noise_index = use_noise ? (int32_t)(frame % 64) : 0;
            c.intensity = config.intensity;
            c.debug_mode = config.debug;
            if (!logged) {
                logged = true;
                report(fmt::format("[GTAO] running {}x{} quality={} denoise={} mul={:.6g} add={:.6g} far_cut={:.6g} tan=({:.3f},{:.3f}) taa={} half={}", size.x, size.y, config.quality, config.denoise, mul, add, c.far_cut, tan_x, tan_y, use_noise, half));
            }
            godot::PackedByteArray push;
            push.resize(sizeof(Constants));
            memcpy(push.ptrw(), &c, sizeof(Constants));

            godot::RID depth_in = buffers -> get_depth_layer(view, false);
            godot::RID color = buffers -> get_color_layer(view, false);
            godot::RID depth_tex = buffers -> get_texture(context, name_depth);
            godot::RID ao_a = buffers -> get_texture(context, name_ao_a);
            godot::RID ao_b = need_b ? buffers -> get_texture(context, name_ao_b) : godot::RID();
            godot::RID edges = buffers -> get_texture(context, name_edges);

            if (!info_logged) {
                info_logged = true;
                auto info = [&](godot::RID rid) {
                    if (!rd -> texture_is_valid(rid)) return std::string("INVALID");
                    auto f = rd -> texture_get_format(rid);
                    return fmt::format("fmt={} usage={} samples={} {}x{}", (int)f -> get_format(), (int64_t)f -> get_usage_bits(), (int)f -> get_samples(), (int)f -> get_width(), (int)f -> get_height());
                };
                report("[GTAO] color: " + info(color) + " | depth: " + info(depth_in) + " | views=" + std::to_string(views) + " msaa=" + std::to_string((int)buffers -> get_msaa_3d()));
            }
            int w16 = (size.x + 15) / 16, w8 = (size.x + 7) / 8;
            int h16 = (size.y + 15) / 16, h8 = (size.y + 7) / 8;
            int aw8 = (ao_size.x + 7) / 8, ah8 = (ao_size.y + 7) / 8, aw16 = (ao_size.x + 15) / 16;
            auto list = rd -> compute_list_begin();
            auto dispatch = [&](const char* label, godot::RID pipeline, godot::RID shader, std::initializer_list<godot::Ref<godot::RDUniform>> uniforms, int x, int y) {
                if (config.debug == 3 && std::string(label) != "final" && std::string(label) != "upsample") return;   // debug 3: only test the color write
                godot::TypedArray<godot::RDUniform> typed;
                for (auto& u : uniforms) typed.push_back(u);
                godot::RID set = godot::UniformSetCacheRD::get_cache(shader, 0, typed);
                if (!rd -> uniform_set_is_valid(set)) {
                    if (!warned) { warned = true; report(std::string("[GTAO] uniform set invalid for pass '") + label + "' (see Godot console for the RD error)", true); }
                    return;
                }
                rd -> compute_list_bind_compute_pipeline(list, pipeline);
                rd -> compute_list_bind_uniform_set(list, set, 0);
                rd -> compute_list_set_push_constant(list, push, push.size());
                rd -> compute_list_dispatch(list, x, y, 1);
                rd -> compute_list_add_barrier(list);
            };

            dispatch("prefilter", prefilter_pipeline, prefilter_shader, {
                sampled(0, depth_in),
                image(1, buffers -> get_texture_slice(context, name_depth, 0, 0, 1, 1)),
                image(2, buffers -> get_texture_slice(context, name_depth, 0, 1, 1, 1)),
                image(3, buffers -> get_texture_slice(context, name_depth, 0, 2, 1, 1)),
                image(4, buffers -> get_texture_slice(context, name_depth, 0, 3, 1, 1)),
                image(5, buffers -> get_texture_slice(context, name_depth, 0, 4, 1, 1))
            }, w16, h16);

            dispatch("main", main_pipeline[config.quality], main_shader[config.quality], {
                sampled(0, depth_tex),
                image(1, ao_a),
                image(2, edges)
            }, aw8, ah8);

            godot::RID current = ao_a, other = ao_b;
            if (half) {
                // Denoise at AO resolution, then upsample (depth-aware) while applying onto the scene color
                for (int i = 0; i < passes; i++) {
                    dispatch("denoise", denoise_pipeline, denoise_shader, {
                        sampled(0, current),
                        sampled(1, edges),
                        image(2, other)
                    }, aw16, ah8);
                    std::swap(current, other);
                }
                dispatch("upsample", upsample_pipeline, upsample_shader, {
                    sampled(0, current),
                    sampled(1, depth_tex),
                    image(2, color)
                }, w8, h8);
            }
            else {
                for (int i = 0; i < passes - 1; i++) {
                    dispatch("denoise", denoise_pipeline, denoise_shader, {
                        sampled(0, current),
                        sampled(1, edges),
                        image(2, other)
                    }, aw16, ah8);
                    std::swap(current, other);
                }

                // Last pass applies the AO onto the scene color
                dispatch("final", final_pipeline, final_shader, {
                    sampled(0, current),
                    sampled(1, edges),
                    image(2, color)
                }, w16, h8);
            }
            rd -> compute_list_end();
        }
    }

    // Managers //
    GTAO::Settings GTAO::get_settings() {
        std::lock_guard<std::mutex> lock(mutex);
        return settings;
    }

    void GTAO::update_settings(const std::function<void(Settings&)>& exec) {
        std::lock_guard<std::mutex> lock(mutex);
        exec(settings);
        settings.quality = std::clamp(settings.quality, 0, 3);
        settings.denoise = std::clamp(settings.denoise, 0, 3);
        settings.radius = std::max(settings.radius, 0.01f);
        settings.intensity = std::clamp(settings.intensity, 0.0f, 1.0f);
        settings.radius_multiplier = std::clamp(settings.radius_multiplier, 0.3f, 3.0f);
        settings.falloff_range = std::clamp(settings.falloff_range, 0.001f, 1.0f);
        settings.sample_distribution_power = std::clamp(settings.sample_distribution_power, 1.0f, 3.0f);
        settings.thin_occluder_compensation = std::clamp(settings.thin_occluder_compensation, 0.0f, 0.7f);
        settings.final_value_power = std::clamp(settings.final_value_power, 0.5f, 5.0f);
        settings.depth_mip_sampling_offset = std::clamp(settings.depth_mip_sampling_offset, 0.0f, 30.0f);
        settings.debug = std::clamp(settings.debug, 0, 3);
        settings.half_res = std::clamp(settings.half_res, 0, 1);
        settings.blur = std::clamp(settings.blur, 0.05f, 5.0f);
    }

    bool GTAO::is_enabled() {
        std::lock_guard<std::mutex> lock(mutex);
        return enabled;
    }

    void GTAO::set_enabled(bool state) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!state && !enabled) return;   // nothing attached, leave the environment untouched
            enabled = state;
        }
        auto compositor = Core::get_compositor();
        if (!compositor.is_valid()) return;
        auto effects = compositor -> get_compositor_effects();
        int found = -1;
        for (int i = 0; i < effects.size(); i++) {
            godot::Ref<godot::CompositorEffect> effect = effects[i];
            if (godot::Object::cast_to<GTAO>(effect.ptr())) { found = i; break; }
        }
        if (state && found < 0) {
            godot::Ref<GTAO> effect;
            effect.instantiate();
            // Post-transparent: with MSAA enabled, anything written earlier to the resolved color is overwritten by the final resolve
            effect -> set_effect_callback_type(godot::CompositorEffect::EFFECT_CALLBACK_TYPE_POST_TRANSPARENT);
            effect -> set_access_resolved_color(true);
            effect -> set_access_resolved_depth(true);
            effects.push_back(effect);
            compositor -> set_compositor_effects(effects);
        }
        else if (!state && found >= 0) {
            effects.remove_at(found);
            compositor -> set_compositor_effects(effects);
        }
    }

    void GTAO::reset() {
        std::lock_guard<std::mutex> lock(mutex);
        enabled = false;
        settings = Settings();
    }
}
#endif
