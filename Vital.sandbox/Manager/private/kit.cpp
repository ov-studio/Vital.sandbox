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

namespace Vital::Manager {
    // Helpers //
    std::string Kit::Internal::to_lower(std::string value) {
        for (auto& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return value;
    }

    const std::string& Kit::Internal::get_version() {
        auto km = Kit::get_singleton();
        if (!km -> version.empty()) return km -> version;
        const std::string checksum_path = fmt::format("{}/{}/checksum.json", std::string(toolkit_base), std::string(toolkit_name));
        std::ifstream file(checksum_path, std::ios::binary);
        if (!file) return km -> version;

        std::string raw{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        rapidjson::Document doc;
        doc.Parse(raw.c_str());
        if (doc.HasParseError() || !doc.IsObject()) return km -> version;
        if (doc.HasMember("version") && doc["version"].IsString()) km -> version = doc["version"].GetString();
        return km -> version;
    }

    const std::string& Kit::Internal::get_content(std::string_view path) {
        auto km = Kit::get_singleton();
        std::string key(path);
        auto it = km -> content_cache.find(key);
        if (it != km -> content_cache.end()) return it -> second;

        std::filesystem::path full = std::filesystem::path(toolkit_base)/toolkit_name/path;
        std::ifstream file(full, std::ios::binary);
        if (!file) {
            static const std::string empty{};
            return empty;
        }
        std::string value{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        if (value.empty()) {
            static const std::string empty{};
            return empty;
        }
        return km -> content_cache.emplace(key, std::move(value)).first -> second;
    }

    rapidjson::Document& Kit::Internal::get_json(const std::string& name) {
        auto km = Kit::get_singleton();
        auto it = km -> json_cache.find(name);
        if (it != km -> json_cache.end()) return it -> second;
        
        rapidjson::Document document;
        document.Parse(get_content(fmt::format("{}.json", name)).c_str());
        if (document.HasParseError()) {
            static rapidjson::Document empty_doc;
            return empty_doc;
        }
        return km -> json_cache.emplace(name, std::move(document)).first -> second;
    }

    bool Kit::Internal::resolve_entry(const std::filesystem::path& root, const std::string& entry, std::filesystem::path& out) {
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

    bool Kit::Internal::verify_tree(const std::filesystem::path& root, const rapidjson::Document& checksum_doc, const std::string& remote_hash) {
        auto km = Kit::get_singleton();
        if (!checksum_doc.IsObject() || !checksum_doc.HasMember("files") || !checksum_doc["files"].IsObject()) { km -> log("error", "verify ~ invalid checksum | reason ~ missing files"); return false; }

        std::unordered_set<std::string> listed;
        const auto& files = checksum_doc["files"];
        for (auto it = files.MemberBegin(); it != files.MemberEnd(); ++it) {
            const std::string rel = it -> name.GetString();
            if (!it -> value.IsObject() || !it -> value.HasMember("sha256") || !it -> value["sha256"].IsString()) { km -> log("error", fmt::format("verify ~ malformed checksum | file ~ {}", rel)); return false; }
            std::filesystem::path full;
            if (!resolve_entry(root, rel, full)) { km -> log("error", fmt::format("verify ~ unsafe path | file ~ {}", rel)); return false; }
            std::error_code ec;
            if (!std::filesystem::is_regular_file(full, ec)) { km -> log("error", fmt::format("verify ~ file missing | file ~ {}", rel)); return false; }
            if (to_lower(Tool::Crypto::hash_file("sha256", full.string())) != to_lower(it -> value["sha256"].GetString())) { km -> log("error", fmt::format("verify ~ checksum mismatch | file ~ {}", rel)); return false; }
            listed.insert(std::filesystem::path(rel).lexically_normal().generic_string());
        }

        std::error_code ec;
        const std::filesystem::path checksum_file = root/"checksum.json";
        if (!std::filesystem::is_regular_file(checksum_file, ec)) { km -> log("error", "verify ~ checksum.json missing from archive"); return false; }
        if (!remote_hash.empty() && to_lower(Tool::Crypto::hash_file("sha256", checksum_file.string())) != to_lower(remote_hash)) { km -> log("error", "verify ~ checksum.json does not match release asset"); return false; }
        for (std::filesystem::recursive_directory_iterator it(root, ec), end; !ec && it != end; it.increment(ec)) {
            const auto status = std::filesystem::symlink_status(it -> path(), ec);
            if (ec) return false;
            if (std::filesystem::is_directory(status)) continue;
            const std::string rel = it -> path().lexically_relative(root).generic_string();
            if (!std::filesystem::is_regular_file(status)) { km -> log("error", fmt::format("verify ~ non-regular file | file ~ {}", rel)); return false; }
            if (rel == "checksum.json") continue;
            if (!listed.count(rel)) { km -> log("error", fmt::format("verify ~ unlisted file | file ~ {}", rel)); return false; }
        }
        if (ec) return false;
        return true;
    }
}

namespace Vital::Manager {
    // Getters //
    const std::string& Kit::get_version() {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_version();
    }

    const std::string& Kit::get_content(std::string_view path) {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_content(path);
    }

    rapidjson::Document& Kit::get_json(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        return Internal::get_json(name);
    }

    std::vector<std::pair<std::string, std::string>> Kit::get_module(const std::string& name) {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<std::pair<std::string, std::string>> result;
        const std::string path = fmt::format("module/{}", name);
        auto& document = Internal::get_json(fmt::format("{}/manifest", path));
        if (document.HasParseError() || !document.IsObject() || !document.HasMember("sources") || !document["sources"].IsArray()) return result;

        for (auto& i : document["sources"].GetArray()) {
            if (!i.IsString()) continue;
            std::string src = i.GetString();
            result.emplace_back(src, std::string(Internal::get_content(fmt::format("{}/{}", path, src))));
        }
        return result;
    }

    std::tuple<std::string, std::string, std::string> Kit::get_release() const {
        std::tuple<std::string, std::string, std::string> result;
        std::string response;
        try { response = Tool::HTTP::get(std::string(toolkit_src)); }
        catch (...) {}
        if (!response.empty()) {
            rapidjson::Document doc;
            doc.Parse(response.c_str());
            if (!doc.HasParseError() && doc.IsObject()) {
                auto& [tag, zip_url, checksum_url] = result;
                if (doc.HasMember("tag_name") && doc["tag_name"].IsString()) tag = doc["tag_name"].GetString();
                if (doc.HasMember("assets") && doc["assets"].IsArray()) {
                    for (auto& asset : doc["assets"].GetArray()) {
                        if (!asset.IsObject() || !asset.HasMember("name") || !asset["name"].IsString() || !asset.HasMember("browser_download_url") || !asset["browser_download_url"].IsString()) continue;
                        std::string asset_name = asset["name"].GetString();
                        std::string asset_url = asset["browser_download_url"].GetString();
                        if (asset_name == "checksum.json") checksum_url = asset_url;
                        if (asset_name.rfind(toolkit_name, 0) == 0 && asset_name.size() > 4 && asset_name.substr(asset_name.size() - 4) == ".zip") zip_url = asset_url;
                    }
                }
            }
        }
        return result;
    }

    rapidjson::Document Kit::get_checksum(const std::string& checksum_url, std::string& checksum_hash) const {
        rapidjson::Document doc;
        if (checksum_url.empty()) return doc;
        
        std::string data;
        try { data = Tool::HTTP::get(checksum_url); }
        catch (...) {}
        if (!data.empty()) {
            checksum_hash = Tool::Crypto::hash("sha256", data);
            doc.Parse(data.c_str());
        }
        return doc;
    }


    // Misc //
    bool Kit::download(const std::string& url, const std::string& dest_path) const {
        try { return Tool::HTTP::download(url, dest_path, download_max_size, 120); }
        catch (const std::exception& e) { log("error", fmt::format("status ~ download failed | reason ~ {}", e.what())); return false; }
    }

    bool Kit::extract(const std::string& zip_path, const std::string& dest_dir) const {
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
            if (!Internal::resolve_entry(stage_dir, entry, out)) return fail(fmt::format("unsafe entry path | entry ~ {}", entry));
            
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
            if ((st.valid & ZIP_STAT_COMP_SIZE) && st.size > (1ull << 20) && st.size / std::max<zip_uint64_t>(st.comp_size, 1) > extract_max_ratio) return fail(fmt::format("compression ratio too high | entry ~ {}", entry));
            total_size += st.size;

            zip_file_t* zf = zip_fopen_index(archive, i, 0);
            if (!zf) return fail(fmt::format("unreadable entry | entry ~ {}", entry));
            std::filesystem::create_directories(out.parent_path(), ec);
            if (ec) { zip_fclose(zf); return fail(fmt::format("directory creation failed | entry ~ {}", entry)); }
            std::ofstream out_f(out, std::ios::binary | std::ios::trunc);
            if (!out_f) { zip_fclose(zf); return fail(fmt::format("write failed | entry ~ {}", entry)); }
            
            char buf[65536];
            zip_int64_t n;
            zip_uint64_t written = 0;
            bool overflow = false;
            while ((n = zip_fread(zf, buf, sizeof(buf))) > 0) {
                if (written + static_cast<zip_uint64_t>(n) > st.size) { overflow = true; break; }
                out_f.write(buf, n);
                written += static_cast<zip_uint64_t>(n);
            }
            zip_fclose(zf);
            out_f.close();
            if (overflow || n < 0 || written != st.size) return fail(fmt::format("entry size mismatch | entry ~ {}", entry));
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

    bool Kit::ensure() {
        const std::string kit_dir = fmt::format("{}/{}", std::string(toolkit_base), std::string(toolkit_name));
        const std::string zip_path = fmt::format("{}/{}.zip", std::string(toolkit_base), std::string(toolkit_name));
        const auto release = get_release();
        const auto& tag = std::get<0>(release);
        const auto& zip_url = std::get<1>(release);
        const auto& checksum_url = std::get<2>(release);
        bool cache_fallback_allowed = false;

        if (tag.empty() || zip_url.empty()) {
            log("error", "status ~ release fetch failed");
            return std::filesystem::exists(kit_dir);
        }
        log("sbox", fmt::format("release ~ {}", tag));

        auto do_download = [&]() -> bool {
            if (checksum_url.empty()) { log("error", "status ~ download aborted | reason ~ release has no checksum.json"); return false; }
            std::string remote_hash;
            auto checksum_doc = get_checksum(checksum_url, remote_hash);
            if (checksum_doc.HasParseError() || !checksum_doc.IsObject()) { log("error", "status ~ download aborted | reason ~ checksum fetch failed"); return false; }

            const std::filesystem::path verify_dir = std::filesystem::path(kit_dir + ".verify");
            const std::filesystem::path old_dir = std::filesystem::path(kit_dir + ".old");
            std::error_code ec;
            auto cleanup = [&]() {
                std::error_code cleanup_ec;
                std::filesystem::remove(zip_path, cleanup_ec);
                std::filesystem::remove_all(verify_dir, cleanup_ec);
            };

            log("sbox", "status ~ downloading");
            if (!download(zip_url, zip_path)) { log("error", "status ~ download failed"); cleanup(); return false; }
            log("sbox", "status ~ extracting");
            if (!extract(zip_path, verify_dir.string())) { log("error", "status ~ extraction failed"); cleanup(); return false; }
            log("sbox", "status ~ verifying");
            if (!Internal::verify_tree(verify_dir, checksum_doc, remote_hash)) { log("error", "status ~ verification failed | toolkit not installed"); cleanup(); return false; }

            std::filesystem::remove_all(old_dir, ec);
            ec.clear();
            const bool had_previous = std::filesystem::exists(kit_dir);
            if (had_previous) {
                std::filesystem::rename(kit_dir, old_dir, ec);
                if (ec) { log("error", "status ~ promotion failed"); cleanup(); return false; }
            }
            std::filesystem::rename(verify_dir, kit_dir, ec);
            if (ec) {
                log("error", "status ~ promotion failed");
                std::error_code restore_ec;
                if (had_previous) std::filesystem::rename(old_dir, kit_dir, restore_ec);
                cleanup();
                return false;
            }

            std::error_code final_ec;
            std::filesystem::remove_all(old_dir, final_ec);
            std::filesystem::remove(zip_path, final_ec);
            {
                std::lock_guard<std::mutex> lock(mutex);
                version.clear();
                content_cache.clear();
                json_cache.clear();
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
                const std::string expected = Internal::to_lower(it -> value["sha256"].GetString());
                if (!Tool::File::exists(kit_dir, rel_path)) { log("warn", fmt::format("file ~ missing | progress ~ {}/{} | path ~ {}", checked, total, rel_path)); return false; }
                if (Internal::to_lower(Tool::Crypto::hash_file("sha256", fmt::format("{}/{}", kit_dir, rel_path))) != expected) { log("warn", fmt::format("checksum ~ mismatch | progress ~ {}/{} | file ~ {}", checked, total, rel_path)); return false; }
                ++checked;
            }

            std::string v;
            {
                std::lock_guard<std::mutex> lock(mutex);
                v = Internal::get_version();
            }
            log("sbox", fmt::format("status ~ cache valid | files ~ {}/{} | version ~ {}", checked, total, v));
            return true;
        };

        auto needs_download = [&]() -> bool {
            if (!std::filesystem::exists(kit_dir)) { log("warn", "status ~ cache missing"); return true; }
            std::string remote_hash;
            auto checksum_doc = get_checksum(checksum_url, remote_hash);
            if (checksum_doc.HasParseError() || !checksum_doc.IsObject()) { 
                log("error", "checksum ~ fetch failed"); 
                cache_fallback_allowed = true; 
                return true; 
            }
    
            std::string local_version;
            {
                std::lock_guard<std::mutex> lock(mutex);
                local_version = Internal::get_version();
            }
            if (local_version != tag) {
                log("warn", fmt::format("version ~ mismatch | local ~ {} | remote ~ {}", local_version, tag));
                cache_fallback_allowed = !local_version.empty();
                return true;
            }
            if (!remote_hash.empty() && Tool::Crypto::hash_file("sha256", fmt::format("{}/checksum.json", kit_dir)) != remote_hash) { log("warn", "checksum ~ tampered"); return true; }
            return !validate_files(checksum_doc);
        };

        if (!needs_download()) return true;
        if (do_download()) return true;
        if (cache_fallback_allowed && std::filesystem::exists(kit_dir)) {
            log("warn", "status ~ update failed | reason ~ keeping existing cached toolkit");
            return true;
        }
        return false;
    }
}

namespace Vital::Tool::Version {
    std::string get_kit() {
        return Vital::Manager::Kit::get_singleton() -> get_version();
    }
}
