#pragma once

#include <RmlUi/Core/FontEngineInterface.h>

namespace gdrml
{

class GDFontInterface : public Rml::FontEngineInterface
{
public:
	GDFontInterface();
	~GDFontInterface();

	bool LoadFontFace(const Rml::String& file_name, bool fallback_face, Rml::Style::FontWeight weight) override;
	bool LoadFontFace(
		const Rml::byte* data,
		int data_size,
		const Rml::String& family,
		Rml::Style::FontStyle style,
		Rml::Style::FontWeight weight,
		bool fallback_face
	) override;
	int GenerateString(
		Rml::FontFaceHandle face_handle,
		Rml::FontEffectsHandle font_effects_handle,
		const Rml::String& string,
		const Rml::Vector2f& position,
		const Rml::Colourb& colour,
		float opacity,
		Rml::GeometryList& geometry
	) override;
};

}
