#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>


using namespace godot;

RmlGodotRenderer::RmlGodotRenderer() = default;
RmlGodotRenderer::~RmlGodotRenderer() = default;

// ----------------------------------------------------
// Geometry (stubbed)
// ----------------------------------------------------

Rml::CompiledGeometryHandle RmlGodotRenderer::CompileGeometry(
    Rml::Span<const Rml::Vertex>,
    Rml::Span<const int>
) {
    return 1; // dummy handle
}

void RmlGodotRenderer::RenderGeometry(
    Rml::CompiledGeometryHandle,
    Rml::Vector2f,
    Rml::TextureHandle
) {
    // no-op for now
}

void RmlGodotRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle) {
    // no-op
}

// ----------------------------------------------------
// Scissor (Godot 4 only supports enable/disable)
// ----------------------------------------------------

void RmlGodotRenderer::EnableScissorRegion(bool) {
    // Godot RenderingServer has no rect scissor
}

void RmlGodotRenderer::SetScissorRegion(Rml::Rectanglei) {
    // rectangle ignored (required by interface)
}

// ----------------------------------------------------
// Textures
// ----------------------------------------------------

Rml::TextureHandle RmlGodotRenderer::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source
) {
    // Not implemented yet
    // Return 0 to indicate failure (RmlUi convention)
    texture_dimensions = Rml::Vector2i(0, 0);
    return 0;
}


Rml::TextureHandle RmlGodotRenderer::GenerateTexture(
    Rml::Span<const Rml::byte> source,
    Rml::Vector2i dimensions
) {
    PackedByteArray bytes;
    bytes.resize(source.size());
    memcpy(bytes.ptrw(), source.data(), source.size());

    Ref<Image> image;
    image.instantiate();
    image->create_from_data(
        dimensions.x,
        dimensions.y,
        false,
        Image::FORMAT_RGBA8,
        bytes
    );

    Ref<ImageTexture> texture = ImageTexture::create_from_image(image);

    Texture t;
    t.rid = texture->get_rid();
    t.size = Vector2i(dimensions.x, dimensions.y);

    Rml::TextureHandle handle = next_texture++;
    textures.emplace(handle, t);

    return handle;
}

void RmlGodotRenderer::ReleaseTexture(Rml::TextureHandle texture) {
    textures.erase(texture);
}
