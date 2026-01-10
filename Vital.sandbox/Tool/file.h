/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: file.h
     Author: vStudio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: File Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Tool/index.h>
#include <Vital.sandbox/Tool/error.h>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/dir_access.hpp>


////////////////////////
// Vital: Tool: File //
///////////////////////

namespace Vital::System::File {

    using namespace godot;

    inline bool exists(std::string& path) {
        return FileAccess::file_exists(to_godot_string(path));
    }

    inline std::streampos size(std::string& path) {
        Ref<FileAccess> f = FileAccess::open(to_godot_string(path), FileAccess::READ);
        if (f.is_null())
            return 0;

        return static_cast<std::streampos>(f->get_length());
    }

    inline bool remove(std::string& path) {
        using namespace godot;

        String p = to_godot_string(path);
    
        // Sandbox safety
        if (!p.begins_with("res://") && !p.begins_with("user://"))
            return false;
    
        // Open directory from absolute path
        Ref<DirAccess> dir = DirAccess::open(p.get_base_dir());
        if (dir.is_null())
            return false;
    
        // Remove file by name
        return dir->remove(p.get_file()) == OK;
    }

    inline std::string read(std::string& path) {
        Ref<FileAccess> f = FileAccess::open(to_godot_string(path), FileAccess::READ);
        if (f.is_null())
            return {};

        String content = f->get_as_text();
        return to_std_string(content);
    }

    inline bool write(std::string& path, const std::string& buffer) {
        Ref<FileAccess> f = FileAccess::open(
            to_godot_string(path),
            FileAccess::WRITE
        );

        if (f.is_null())
            return false;

        f->store_string(to_godot_string(buffer));
        return true;
    }

    inline std::vector<std::string> contents(
        std::string& path,
        bool fetchDirs = false
    ) {
        std::vector<std::string> out;
        String p = to_godot_string(path);
        PackedStringArray files = FileAccess::get_files_at_path(p);
        for (int i = 0; i < files.size(); ++i)
            out.emplace_back(to_std_string(files[i]));
        if (fetchDirs) {
            PackedStringArray dirs = FileAccess::get_directories_at_path(p);
            for (int i = 0; i < dirs.size(); ++i)
                out.emplace_back(to_std_string(dirs[i]));
        }
        return out;
    }
}