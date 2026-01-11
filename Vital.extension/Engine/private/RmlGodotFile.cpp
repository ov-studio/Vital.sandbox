#include <Vital.extension/Engine/public/RmlGodotFile.h>

Rml::FileHandle RmlGodotFile::Open(const Rml::String& path) {
    auto* file = new std::ifstream(path.c_str(), std::ios::binary);
    if (!file->is_open()) {
        delete file;
        return 0;
    }
    return reinterpret_cast<Rml::FileHandle>(file);
}

void RmlGodotFile::Close(Rml::FileHandle file) {
    delete reinterpret_cast<std::ifstream*>(file);
}

size_t RmlGodotFile::Read(void* buffer, size_t size, Rml::FileHandle file) {
    auto* f = reinterpret_cast<std::ifstream*>(file);
    f->read((char*)buffer, size);
    return f->gcount();
}

bool RmlGodotFile::Seek(Rml::FileHandle file, long offset, int origin) {
    auto* f = reinterpret_cast<std::ifstream*>(file);
    std::ios_base::seekdir dir =
        origin == SEEK_SET ? std::ios::beg :
        origin == SEEK_CUR ? std::ios::cur :
                             std::ios::end;
    f->seekg(offset, dir);
    return true;
}

size_t RmlGodotFile::Tell(Rml::FileHandle file) {
    return (size_t)reinterpret_cast<std::ifstream*>(file)->tellg();
}
