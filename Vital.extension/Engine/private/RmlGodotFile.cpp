#include <Vital.extension/Engine/public/RmlGodotFile.h>
#include <godot_cpp/classes/file_access.hpp>

using namespace godot;

Rml::FileHandle RmlGodotFile::Open(const Rml::String& path) {
    Ref<FileAccess> file = FileAccess::open(path.c_str(), FileAccess::READ);
    if (!file.is_valid()) {
        UtilityFunctions::print("RmlGodotFile::Open failed: ", path.c_str());
        return 0;
    }

    Rml::FileHandle handle = next_handle++;
    open_files[handle] = { file };
    return handle;
}

void RmlGodotFile::Close(Rml::FileHandle file) {
    auto it = open_files.find(file);
    if (it != open_files.end()) {
        it->second.file.unref();
        open_files.erase(it);
    }
}

size_t RmlGodotFile::Read(void* buffer, size_t size, Rml::FileHandle file) {
    auto it = open_files.find(file);
    if (it == open_files.end()) return 0;

    return it->second.file->get_buffer((uint8_t*)buffer, size);
}

bool RmlGodotFile::Seek(Rml::FileHandle file, long offset, int origin) {
    auto it = open_files.find(file);
    if (it == open_files.end())
        return false;

    auto& f = it->second.file;

    switch (origin) {
        case SEEK_SET:
            f->seek(offset);
            break;

        case SEEK_CUR:
            f->seek(f->get_position() + offset);
            break;

        case SEEK_END:
            f->seek_end(offset);
            break;

        default:
            return false;
    }

    return true;
}
size_t RmlGodotFile::Tell(Rml::FileHandle file) {
    auto it = open_files.find(file);
    if (it == open_files.end())
        return 0;

    return static_cast<size_t>(it->second.file->get_position());
}