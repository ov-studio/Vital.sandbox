#pragma once

#include <Vital.sandbox/vital.hpp>

#include <RmlUi/Core/RenderInterface.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <unordered_map>

struct GodotGeometry {
    godot::RID mesh;
};

struct GodotTexture {
    godot::RID rid;
    godot::Vector2i size;
};

class RmlGodotRenderer final : public Rml::RenderInterface {
public:
    godot::RID canvas;

    // Geometry
    Rml::CompiledGeometryHandle CompileGeometry(
        Rml::Span<const Rml::Vertex> vertices,
        Rml::Span<const int> indices
    ) override;

    void RenderGeometry(
        Rml::CompiledGeometryHandle geometry,
        Rml::Vector2f translation,
        Rml::TextureHandle texture
    ) override;

    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    // Textures (file)
    Rml::TextureHandle LoadTexture(
        Rml::Vector2i& texture_dimensions,
        const Rml::String& source
    ) override;

    // Textures (memory)  ✅ REQUIRED IN RMLUI v6
    Rml::TextureHandle GenerateTexture(
        Rml::Span<const Rml::byte> source,
        Rml::Vector2i dimensions
    ) override;

    void ReleaseTexture(Rml::TextureHandle texture) override;

    // Scissor
    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

private:
    std::unordered_map<Rml::CompiledGeometryHandle, GodotGeometry> geometries;
    std::unordered_map<Rml::TextureHandle, GodotTexture> textures;

    Rml::CompiledGeometryHandle next_geometry = 1;
    Rml::TextureHandle next_texture = 1;
};