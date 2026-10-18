/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Manager: kit.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Kit Manager
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#include <Vital.sandbox/Manager/public/kit.h>


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager::Kit {
    static constexpr zip_int64_t extract_max_entries = 100000;
    static constexpr zip_uint64_t extract_max_entry_size = 2ull * 1024 * 1024 * 1024;
    static constexpr zip_uint64_t extract_max_total_size = 4ull * 1024 * 1024 * 1024;
    
    bool download(const std::string& url, const std::string& dest_path) {
        std::string data;
        try { data = Tool::HTTP::get(url, {}, 120); }
        catch (const std::exception& e) { log("error", fmt::format("status ~ download failed | reason ~ {}", e.what())); return false; }
        if (data.empty()) return false;
        
        std::filesystem::create_directories(std::filesystem::path(dest_path).parent_path());
        std::ofstream out(dest_path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out.write(data.data(), static_cast<std::streamsize>(data.size()));
        return out.good();
    }

    static bool resolve_entry(const std::filesystem::path& root, const std::string& entry, std::filesystem::path& out) {
        if (entry.empty() || entry.find('\0') != std::string::npos) return false;
        if (entry.find('\\') != std::string::npos || entry.find(':') != std::string::npos || entry.front() == '/') return false;
        std::filesystem::path relative(entry);
        if (relative.is_absolute() || relative.has_root_name() || relative.has_root_directory()) return false;
        for (const auto& part : relative) {
            if (part.empty() || part == "." || part == "..") return false;
        }
        
        std::error_code ec;
        auto base = std::filesystem::weakly_canonical(root, ec);
        if (ec) return false;
        auto target = std::filesystem::weakly_canonical(base/relative, ec);
        if (ec) return false;
        auto rel = target.lexically_relative(base);
        if (rel.empty() || rel == "." || rel.begin() -> string() == "..") return false;
        out = target;
        return true;
    }

    bool extract(const std::string& zip_path, const std::string& dest_dir) {
        int err = 0;
        zip_t* archive = zip_open(zip_path.c_str(), ZIP_RDONLY, &err);
        if (!archive) return false;

        const std::filesystem::path final_dir(dest_dir);
        std::filesystem::path stage_dir = final_dir;
        stage_dir += ".staging";
        std::error_code ec;
        std::filesystem::remove_all(stage_dir, ec);
        ec.clear();
        std::filesystem::create_directories(stage_dir, ec);
        if (ec) { zip_close(archive); return false; }

        auto fail = [&](const std::string& reason) -> bool {
            log("error", fmt::format("status ~ extraction rejected | reason ~ {}", reason));
            zip_close(archive);
            std::error_code cleanup_ec;
            std::filesystem::remove_all(stage_dir, cleanup_ec);
            return false;
        };

        zip_int64_t count = zip_get_num_entries(archive, 0);
        if (count < 0 || count > extract_max_entries) return fail("invalid entry count");
        zip_uint64_t total_size = 0;
        for (zip_int64_t i = 0; i < count; ++i) {
            const char* entry_name = zip_get_name(archive, i, 0);
            if (!entry_name) return fail("unreadable entry name");
            std::string entry(entry_name);
            if (entry.empty()) return fail("empty entry name");
            if (entry.back() == '/') continue;
            std::filesystem::path out;
            if (!resolve_entry(stage_dir, entry, out)) return fail(fmt::format("unsafe entry path | entry ~ {}", entry));
            
            zip_uint8_t opsys = 0;
            zip_uint32_t attributes = 0;
            if (zip_file_get_external_attributes(archive, i, 0, &opsys, &attributes) == 0 && opsys == ZIP_OPSYS_UNIX) {
                const zip_uint32_t mode = attributes >> 16;
                if (mode != 0 && (mode & 0170000) != 0100000) return fail(fmt::format("non-regular entry | entry ~ {}", entry));
            }

            zip_stat_t st;
            zip_stat_init(&st);
            if (zip_stat_index(archive, i, 0, &st) != 0 || !(st.valid & ZIP_STAT_SIZE)) return fail(fmt::format("unknown entry size | entry ~ {}", entry));
            if (st.size > extract_max_entry_size || st.size > extract_max_total_size - total_size) return fail(fmt::format("entry too large | entry ~ {}", entry));
            total_size += st.size;

            zip_file_t* zf = zip_fopen_index(archive, i, 0);
            if (!zf) return fail(fmt::format("unreadable entry | entry ~ {}", entry));
            std::string contents;
            contents.reserve(static_cast<size_t>(st.size));
            char buf[65536];
            zip_int64_t n;
            bool overflow = false;
            while ((n = zip_fread(zf, buf, sizeof(buf))) > 0) {
                if (contents.size() + static_cast<size_t>(n) > st.size) { overflow = true; break; }
                contents.append(buf, static_cast<size_t>(n));
            }
            zip_fclose(zf);
            if (overflow || n < 0 || contents.size() != st.size) return fail(fmt::format("entry size mismatch | entry ~ {}", entry));

            std::filesystem::create_directories(out.parent_path(), ec);
            if (ec) return fail(fmt::format("directory creation failed | entry ~ {}", entry));
            std::ofstream out_f(out, std::ios::binary | std::ios::trunc);
            if (!out_f) return fail(fmt::format("write failed | entry ~ {}", entry));
            out_f.write(contents.data(), static_cast<std::streamsize>(contents.size()));
            out_f.close();
            if (!out_f.good()) return fail(fmt::format("write failed | entry ~ {}", entry));
        }
        zip_close(archive);

        std::filesystem::remove_all(final_dir, ec);
        ec.clear();
        std::filesystem::rename(stage_dir, final_dir, ec);
        if (ec) {
            std::error_code cleanup_ec;
            std::filesystem::remove_all(stage_dir, cleanup_ec);
            return false;
        }
        return true;
    }

    bool ensure() {
        const std::string kit_dir = fmt::format("{}/{}", std::string(Internal::toolkit_base), std::string(Internal::toolkit_name));
        const std::string zip_path = fmt::format("{}/{}.zip", std::string(Internal::toolkit_base), std::string(Internal::toolkit_name));
        auto [tag, zip_url, checksum_url] = fetch_release();

        if (tag.empty() || zip_url.empty()) {
            log("error", "status ~ release fetch failed");
            return std::filesystem::exists(kit_dir);
        }
        log("sbox", fmt::format("release ~ {}", tag));

        auto do_download = [&]() -> bool {
            std::filesystem::remove_all(kit_dir);
            log("sbox", "status ~ downloading");
            if (!download(zip_url, zip_path)) { log("error", "status ~ download failed"); return false; }
            log("sbox", "status ~ extracting");
            if (!extract(zip_path, kit_dir)) { log("error", "status ~ extraction failed"); return false; }
            std::filesystem::remove(zip_path);
            {
                std::lock_guard<std::mutex> lock(Internal::mutex);
                Internal::version.clear();
                Internal::content_cache.clear();
                Internal::json_cache.clear();
            }
            log("sbox", fmt::format("status ~ ready | version ~ {}", get_version()));
            return true;
        };

        auto validate_files = [&](const rapidjson::Document& checksum_doc) -> bool {
            if (!checksum_doc.IsObject() || !checksum_doc.HasMember("files") || !checksum_doc["files"].IsObject()) { log("warn", "checksum ~ invalid | reason ~ missing files"); return false; }
            const auto& files = checksum_doc["files"];
            const int total = static_cast<int>(files.MemberCount());
            int checked = 0;
            for (auto it = files.MemberBegin(); it != files.MemberEnd(); ++it) {
                const std::string rel_path = it -> name.GetString();
                if (!it -> value.IsObject() || !it -> value.HasMember("sha256") || !it -> value["sha256"].IsString()) { log("warn", fmt::format("checksum ~ malformed | file ~ {}", rel_path)); return false; }
                const std::string expected = it -> value["sha256"].GetString();
                if (!Tool::File::exists(kit_dir, rel_path)) { log("warn", fmt::format("file ~ missing | progress ~ {}/{} | path ~ {}", checked, total, rel_path)); return false; }
                if (Tool::Crypto::hash_file("sha256", fmt::format("{}/{}", kit_dir, rel_path)) != expected) { log("warn", fmt::format("checksum ~ mismatch | progress ~ {}/{} | file ~ {}", checked, total, rel_path)); return false; }
                ++checked;
            }
            std::string v;
            {
                std::lock_guard<std::mutex> lock(Internal::mutex);
                v = Internal::get_version();
            }
            log("sbox", fmt::format("status ~ cache valid | files ~ {}/{} | version ~ {}", checked, total, v));
            return true;
        };

        auto needs_download = [&]() -> bool {
            if (!std::filesystem::exists(kit_dir)) { log("warn", "status ~ cache missing"); return true; }
            std::string remote_hash;
            auto checksum_doc = fetch_checksum(checksum_url, remote_hash);
            if (checksum_doc.HasParseError() || !checksum_doc.IsObject()) { log("error", "checksum ~ fetch failed"); return true; }
            std::string local_version;
            {
                std::lock_guard<std::mutex> lock(Internal::mutex);
                local_version = Internal::get_version();
            }
            if (local_version != tag) { log("warn", fmt::format("version ~ mismatch | local ~ {} | remote ~ {}", local_version, tag)); return true; }
            if (!remote_hash.empty() && Tool::Crypto::hash_file("sha256", fmt::format("{}/checksum.json", kit_dir)) != remote_hash) { log("warn", "checksum ~ tampered"); return true; }
            return !validate_files(checksum_doc);
        };

        return needs_download() ? do_download() : true;
    }
}