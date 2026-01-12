#pragma once
#include <RmlUi/Core/FileInterface.h>
#include <Vital.sandbox/vital.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <map>
#include <memory>

class RmlGodotFile final : public Rml::FileInterface {
    public:
        Rml::FileHandle Open(const Rml::String& path) override;
        void Close(Rml::FileHandle file) override;
        size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
        bool Seek(Rml::FileHandle file, long offset, int origin) override;
        size_t Tell(Rml::FileHandle file) override;
    
    private:
        struct FileData {
            godot::Ref<godot::FileAccess> file;
        };
        std::map<Rml::FileHandle, FileData> open_files;
        Rml::FileHandle next_handle = 1;
    };