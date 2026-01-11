#pragma once

#include <Vital.sandbox/vital.hpp>

#include <RmlUi/Core/RenderInterface.h>
#include <godot_cpp/classes/rendering_server.hpp>
#include <unordered_map>


class RmlGodotRenderer final : public Rml::RenderInterface {
public:
    RmlGodotRenderer();
    ~RmlGodotRenderer() override;

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

    // Scissor
    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

    // Textures
    Rml::TextureHandle LoadTexture(
        Rml::Vector2i& texture_dimensions,
        const Rml::String& source
    ) override;
    

    Rml::TextureHandle GenerateTexture(
        Rml::Span<const Rml::byte> source,
        Rml::Vector2i dimensions
    ) override;

    void ReleaseTexture(Rml::TextureHandle texture) override;
private:
    struct Texture {
        godot::RID rid;
        godot::Vector2i size;
    };

    Rml::TextureHandle next_texture = 1;
    std::unordered_map<Rml::TextureHandle, Texture> textures;
};
