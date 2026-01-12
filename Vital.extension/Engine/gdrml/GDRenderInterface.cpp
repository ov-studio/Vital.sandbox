#include "GDRenderInterface.hpp"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cstring>

using namespace gdrml;
using namespace godot;

// ------------------------------------------------------------

GDRenderInterface::GDRenderInterface() = default;

GDRenderInterface::~GDRenderInterface() {
	for (Geometry* g : m_geometries)
		delete g;
	m_geometries.clear();
	m_textures.clear();
}

// ------------------------------------------------------------

void GDRenderInterface::SetCanvasItem(RID canvas_item) {
	m_canvas_item = canvas_item;
}

// ------------------------------------------------------------
// GEOMETRY (IMMEDIATE MODE BACKING STORE)
// ------------------------------------------------------------

Rml::CompiledGeometryHandle GDRenderInterface::CompileGeometry(
	Rml::Span<const Rml::Vertex> vertices,
	Rml::Span<const int> indices
) {
	Geometry* geom = new Geometry;
	geom->vertices.assign(vertices.begin(), vertices.end());
	geom->indices.assign(indices.begin(), indices.end());

	m_geometries.push_back(geom);
	return reinterpret_cast<Rml::CompiledGeometryHandle>(geom);
}

void GDRenderInterface::RenderGeometry(
	Rml::CompiledGeometryHandle geometry_handle,
	Rml::Vector2f translation,
	Rml::TextureHandle texture
) {
	if (!geometry_handle)
		return;

	Geometry* geom = reinterpret_cast<Geometry*>(geometry_handle);
	RenderingServer* rs = RenderingServer::get_singleton();

	const int vcount = (int)geom->vertices.size();
	const int icount = (int)geom->indices.size();

	UtilityFunctions::print("Rendering geometry: ", vcount, " vertices");

	PackedVector2Array positions;
	PackedVector2Array uvs;
	PackedColorArray colors;
	PackedInt32Array indices;

	positions.resize(vcount);
	uvs.resize(vcount);
	colors.resize(vcount);
	indices.resize(icount);

	for (int i = 0; i < vcount; i++) {
		const Rml::Vertex& v = geom->vertices[i];

		positions[i] = Vector2(
			translation.x + v.position.x,
			translation.y + v.position.y
		);

		uvs[i] = Vector2(v.tex_coord.x, v.tex_coord.y);

		colors[i] = Color(
			v.colour.red / 255.0f,
			v.colour.green / 255.0f,
			v.colour.blue / 255.0f,
			v.colour.alpha / 255.0f
		);
	}

	for (int i = 0; i < icount; i++)
		indices[i] = geom->indices[i];

	Texture2D* tex = reinterpret_cast<Texture2D*>(texture);

	if (tex) {
		rs->canvas_item_add_triangle_array(
			m_canvas_item,
			indices,
			positions,
			colors,
			uvs,
			{},
			{},
			tex->get_rid()
		);
	} else {
		rs->canvas_item_add_triangle_array(
			m_canvas_item,
			indices,
			positions,
			colors,
			uvs
		);
	}
}

void GDRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry_handle) {
	if (!geometry_handle)
		return;

	Geometry* geom = reinterpret_cast<Geometry*>(geometry_handle);
	auto it = std::find(m_geometries.begin(), m_geometries.end(), geom);
	if (it != m_geometries.end()) {
		delete geom;
		m_geometries.erase(it);
	}
}

// ------------------------------------------------------------
// TEXTURES
// ------------------------------------------------------------

Rml::TextureHandle GDRenderInterface::LoadTexture(
	Rml::Vector2i& texture_dimensions,
	const Rml::String& source
) {
	Ref<Texture2D> tex = ResourceLoader::get_singleton()->load(String(source.c_str()));
	if (tex.is_null())
		return 0;

	texture_dimensions = Rml::Vector2i(tex->get_width(), tex->get_height());
	m_textures.push_back(tex);

	return reinterpret_cast<Rml::TextureHandle>(tex.ptr());
}

Rml::TextureHandle GDRenderInterface::GenerateTexture(
	Rml::Span<const Rml::byte> source,
	Rml::Vector2i source_dimensions
) {
	Ref<Image> image;
	image.instantiate();

	PackedByteArray bytes;
	bytes.resize(source.size());
	std::memcpy(bytes.ptrw(), source.data(), source.size());

	image->set_data(
		source_dimensions.x,
		source_dimensions.y,
		false,
		Image::FORMAT_RGBA8,
		bytes
	);

	Ref<Texture2D> tex = ImageTexture::create_from_image(image);
	m_textures.push_back(tex);

	return reinterpret_cast<Rml::TextureHandle>(tex.ptr());
}

void GDRenderInterface::ReleaseTexture(Rml::TextureHandle texture) {
	Texture2D* tex = reinterpret_cast<Texture2D*>(texture);
	if (!tex)
		return;

	auto it = std::find_if(
		m_textures.begin(),
		m_textures.end(),
		[tex](const Ref<Texture2D>& r) { return r.ptr() == tex; }
	);

	if (it != m_textures.end())
		m_textures.erase(it);
}

// ------------------------------------------------------------
// SCISSOR
// ------------------------------------------------------------

void GDRenderInterface::EnableScissorRegion(bool enable) {
	RenderingServer::get_singleton()->canvas_item_set_clip(m_canvas_item, enable);
}

void GDRenderInterface::SetScissorRegion(Rml::Rectanglei region) {
	RenderingServer::get_singleton()->canvas_item_set_custom_rect(
		m_canvas_item,
		true,
		Rect2(
			region.Left(),
			region.Top(),
			region.Width(),
			region.Height()
		)
	);
}
