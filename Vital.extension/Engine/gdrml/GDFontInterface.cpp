#pragma once

#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Span.h>

namespace gdrml {

class GDFontInterface final : public Rml::FontEngineInterface {
public:
	GDFontInterface() = default;
	~GDFontInterface() override = default;

	// Lifecycle
	void Initialize() override;
	void Shutdown() override;

	// Load from file
	bool LoadFontFace(
		const Rml::String& file_name,
		int face_index,
		bool fallback_face,
		Rml::Style::FontWeight weight
	) override;

	// Load from memory
	bool LoadFontFace(
		Rml::Span<const Rml::byte> data,
		int face_index,
		const Rml::String& family,
		Rml::Style::FontStyle style,
		Rml::Style::FontWeight weight,
		bool fallback_face
	) override;

	// Resolve font handle
	Rml::FontFaceHandle GetFontFaceHandle(
		const Rml::String& family,
		Rml::Style::FontStyle style,
		Rml::Style::FontWeight weight,
		int size
	) override;

	// Font effects
	Rml::FontEffectsHandle PrepareFontEffects(
		Rml::FontFaceHandle handle,
		const Rml::FontEffectList& font_effects
	) override;

	// Metrics
	const Rml::FontMetrics& GetFontMetrics(
		Rml::FontFaceHandle handle
	) override;

	// String width
	int GetStringWidth(
		Rml::FontFaceHandle handle,
		Rml::StringView string,
		const Rml::TextShapingContext& text_shaping_context,
		Rml::Character prior_character
	) override;

	// Text mesh generation
	int GenerateString(
		Rml::RenderManager& render_manager,
		Rml::FontFaceHandle face_handle,
		Rml::FontEffectsHandle font_effects_handle,
		Rml::StringView string,
		Rml::Vector2f position,
		Rml::ColourbPremultiplied colour,
		float opacity,
		const Rml::TextShapingContext& text_shaping_context,
		Rml::TexturedMeshList& mesh_list
	) override;

	// Versioning
	int GetVersion(Rml::FontFaceHandle handle) override;

	// Cleanup
	void ReleaseFontResources() override;

private:
	Rml::FontMetrics dummy_metrics{};
};

} // namespace gdrml
