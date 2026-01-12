#pragma once

#include <vector>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Types.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

namespace gdrml {

class GDFileInterface : public Rml::FileInterface
{
public:
	GDFileInterface();
	~GDFileInterface();

	Rml::FileHandle Open(const Rml::String& path) override;
	void Close(Rml::FileHandle file) override;
	size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
	bool Seek(Rml::FileHandle file, long offset, int origin) override;
	size_t Tell(Rml::FileHandle file) override;

private:
	// guh (can you tell I don't usually program C++?)
	std::vector<Ref<FileAccess>> m_OpenHandles;
};

}
