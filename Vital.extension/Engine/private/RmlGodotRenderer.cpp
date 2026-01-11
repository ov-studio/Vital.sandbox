#include <Vital.extension/Engine/public/RmlGodotRenderer.h>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/resource_loader.hpp>


using namespace godot;

// =====================================================
// Geometry
// =====================================================

Rml::CompiledGeometryHandle RmlGodotRenderer::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int> indices
) {
    PackedVector2Array positions;
    PackedColorArray colors;
    PackedVector2Array uvs;
    PackedInt32Array index_array;

    for (const auto& v : vertices) {
        positions.push_back(Vector2(v.position.x, v.position.y));
        colors.push_back(Color(
            v.colour.red   / 255.f,
            v.colour.green / 255.f,
            v.colour.blue  / 255.f,
            v.colour.alpha / 255.f
        ));
        uvs.push_back(Vector2(v.tex_coord.x, v.tex_coord.y));
    }

    for (int i : indices)
        index_array.push_back(i);

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = positions;
    arrays[Mesh::ARRAY_COLOR] = colors;
    arrays[Mesh::ARRAY_TEX_UV] = uvs;
    arrays[Mesh::ARRAY_INDEX] = index_array;

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    GodotGeometry geo;
    geo.mesh = mesh->get_rid();

    auto handle = next_geometry++;
    geometries[handle] = geo;
    return handle;
}

void RmlGodotRenderer::RenderGeometry(
    Rml::CompiledGeometryHandle geometry,
    Rml::Vector2f translation,
    Rml::TextureHandle /*texture*/
) {
    auto it = geometries.find(geometry);
    if (it == geometries.end())
        return;

    Transform2D transform;
    transform.set_origin(Vector2(translation.x, translation.y));

    RenderingServer::get_singleton()->canvas_item_add_mesh(
        canvas,
        it->second.mesh,
        transform
    );
}

void RmlGodotRenderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    auto it = geometries.find(geometry);
    if (it == geometries.end())
        return;

    RenderingServer::get_singleton()->free_rid(it->second.mesh);
    geometries.erase(it);
}

// =====================================================
// Textures (file)
// =====================================================

Rml::TextureHandle RmlGodotRenderer::LoadTexture(
    Rml::Vector2i& texture_dimensions,
    const Rml::String& source
) {
    Ref<Image> image = ResourceLoader::get_singleton()->load(
        String(source.c_str())
    );

    if (image.is_null()) {
        texture_dimensions = {0, 0};
        return 0;
    }

    texture_dimensions = {
        image->get_width(),
        image->get_height()
    };

    Ref<ImageTexture> texture = ImageTexture::create_from_image(image);

    GodotTexture gt;
    gt.rid = texture->get_rid();
    gt.size = Vector2i(
        texture_dimensions.x,
        texture_dimensions.y
    );

    auto handle = next_texture++;
    textures[handle] = gt;
    return handle;
}

// =====================================================
// Textures (memory)  ✅ REQUIRED
// =====================================================

Rml::TextureHandle RmlGodotRenderer::GenerateTexture(
    Rml::Span<const Rml::byte> source,
    Rml::Vector2i dimensions
) {
    // Copy raw RGBA8 data into Godot array
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

    GodotTexture gt;
    gt.rid = texture->get_rid();
    gt.size = Vector2i(dimensions.x, dimensions.y);

    Rml::TextureHandle handle = next_texture++;
    textures[handle] = gt;
    return handle;
}


void RmlGodotRenderer::ReleaseTexture(Rml::TextureHandle texture) {
    auto it = textures.find(texture);
    if (it == textures.end())
        return;

    RenderingServer::get_singleton()->free_rid(it->second.rid);
    textures.erase(it);
}

// =====================================================
// Scissor
// =====================================================

void RmlGodotRenderer::EnableScissorRegion(bool enable) {
    RenderingServer::get_singleton()->canvas_item_set_clip(canvas, enable);
}

void RmlGodotRenderer::SetScissorRegion(Rml::Rectanglei) {
    // Godot 4 does not support scissor rectangles via RenderingServer
}