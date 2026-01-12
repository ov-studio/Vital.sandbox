#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Vertex.h>

#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include <vector>

namespace gdrml {

class GDRenderInterface final : public Rml::RenderInterface {
public:
	GDRenderInterface();
	~GDRenderInterface() override;

	// Godot integration
	void SetCanvasItem(godot::RID canvas_item);

	// --- Required by RmlUi (immediate-mode implementation) ---
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

	Rml::TextureHandle LoadTexture(
		Rml::Vector2i& texture_dimensions,
		const Rml::String& source
	) override;

	Rml::TextureHandle GenerateTexture(
		Rml::Span<const Rml::byte> source,
		Rml::Vector2i source_dimensions
	) override;

	void ReleaseTexture(Rml::TextureHandle texture) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(Rml::Rectanglei region) override;

private:
	struct Geometry {
		std::vector<Rml::Vertex> vertices;
		std::vector<int> indices;
	};

	godot::RID m_canvas_item;
	std::vector<Geometry*> m_geometries;
	std::vector<godot::Ref<godot::Texture2D>> m_textures;
};

} // namespace gdrml
