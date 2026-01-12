#include "GDFileInterface.hpp"
#include "Conversion.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/godot.hpp>
#include <algorithm>

using namespace gdrml;
using namespace godot;

GDFileInterface::GDFileInterface()
{
	// empty for now
}

GDFileInterface::~GDFileInterface()
{
	// close all still open handles
	for (Ref<godot::FileAccess> fHandle : m_OpenHandles)
	{
		if (!fHandle.is_valid() || fHandle.is_null())
		{
			continue;
		}

		if (fHandle.ptr()->is_open())
		{
			fHandle.ptr()->close();
		}
	}

	// clear out the vector
	m_OpenHandles.clear();
}

Rml::FileHandle GDFileInterface::Open(const Rml::String& path)
{
	String gdPath = Conversion::stringToGodot(path);

	if (FileAccess::file_exists(gdPath))
	{
		// we have to hold it ourselves or we might lose it due to scope! hooray!!!!!!!!!
		m_OpenHandles.push_back(FileAccess::open(gdPath, FileAccess::READ));
		// return the pointer within
		return reinterpret_cast<Rml::FileHandle>(m_OpenHandles.back().ptr());
	}
	else
	{
		return reinterpret_cast<Rml::FileHandle>(nullptr);
	}
}

void GDFileInterface::Close(Rml::FileHandle file)
{
	// pointers :smile:
	FileAccess* handle = reinterpret_cast<FileAccess*>(file);
	
	if (handle == nullptr)
	{
		UtilityFunctions::printerr("GDRML: Attempted to close invalid FileHandle!");
		return;
	}

	if (handle->is_open())
	{
		handle->close();
	}
	else
	{
		UtilityFunctions::print("GDRML: INF: FileHandle was already closed?");
	}

	// locate this pointer within our list and remove it (disgusting!!)
	if (std::find(m_OpenHandles.begin(), m_OpenHandles.end(), handle) != m_OpenHandles.end())
	{
		std::remove(m_OpenHandles.begin(), m_OpenHandles.end(), handle);
	}
}

size_t GDFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file)
{
	FileAccess* handle = reinterpret_cast<FileAccess*>(file);

	if (handle == nullptr)
	{
		UtilityFunctions::printerr("GDRML: invalid FileHandle!");
		return 0;
	}

	uint64_t amount = handle->get_buffer(static_cast<uint8_t*>(buffer), static_cast<uint64_t>(size));

	return static_cast<size_t>(amount);
}

bool GDFileInterface::Seek(Rml::FileHandle file, long offset, int origin)
{
	FileAccess* handle = reinterpret_cast<FileAccess*>(file);

	if (handle == nullptr)
	{
		UtilityFunctions::printerr("GDRML: invalid FileHandle!");
		return false;
	}

	switch (origin)
	{
	case SEEK_SET:
		handle->seek(offset);
		break;
	case SEEK_CUR:
		handle->seek(handle->get_position() + offset);
		break;
	case SEEK_END:
		handle->seek_end(offset);
		break;
	default:
		UtilityFunctions::printerr("GDRML: Invalid seek position! must be SEEK_SET(0), SEEK_CUR(1), or SEEK_END(2)!");
		return false;
	}

	// we just assume it was successful, seek returns nothing.
	return true;
}

size_t GDFileInterface::Tell(Rml::FileHandle file)
{
	FileAccess* handle = reinterpret_cast<FileAccess*>(file);

	if (handle == nullptr)
	{
		UtilityFunctions::printerr("GDRML: invalid FileHandle!");
		return 0;
	}
	
	// this is a uint64_t so uh. yeah. fun. sick.
	return static_cast<size_t>(handle->get_position());
}
