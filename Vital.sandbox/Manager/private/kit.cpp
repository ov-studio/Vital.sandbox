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
#include <Vital.sandbox/Engine/public/console.h>
#include <zip.h>


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager::Kit {
    static const auto log = [](const std::string& message) {
        Vital::print("sbox", fmt::format("[Vital.kit] {}", message));
    };

    bool download(const std::string& url, const std::string& dest_path) {
        std::string data;
        try { data = Tool::Rest::get(url, {}, 120); }
        catch (const std::exception& e) { log(fmt::format("Status ~ Download Failed | Reason ~ {}", e.what())); return false; }
        if (data.empty()) return false;
        std::filesystem::create_directories(std::filesystem::path(dest_path).parent_path());
        std::ofstream out(dest_path, std::ios::binary | std::ios::trunc);
        if (!out) return false;
        out.write(data.data(), static_cast<std::streamsize>(data.size()));
        return out.good();
    }

    bool extract(const std::string& zip_path, const std::string& dest_dir) {
        int err = 0;
        zip_t* archive = zip_open(zip_path.c_str(), ZIP_RDONLY, &err);
        if (!archive) return false;
        zip_int64_t count = zip_get_num_entries(archive, 0);
        for (zip_int64_t i = 0; i < count; ++i) {
            const char* entry_name = zip_get_name(archive, i, 0);
            if (!entry_name) continue;
            std::string entry(entry_name);
            if (entry.empty() || entry.back() == '/') continue;
            zip_file_t* zf = zip_fopen_index(archive, i, 0);
            if (!zf) continue;
            std::string contents;
            char buf[65536];
            zip_int64_t n;
            while ((n = zip_fread(zf, buf, sizeof(buf))) > 0)
                contents.append(buf, static_cast<size_t>(n));
            zip_fclose(zf);
            std::filesystem::path out = std::filesystem::path(dest_dir)/entry;
            std::filesystem::create_directories(out.parent_path());
            std::ofstream out_f(out, std::ios::binary | std::ios::trunc);
            if (out_f) out_f.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        }
        zip_close(archive);
        return true;
    }

    bool ensure() {
        const std::string kit_dir  = std::string(cache_base) + "/" + std::string(kit_name);
        const std::string zip_path = std::string(cache_base) + "/" + std::string(kit_name) + ".zip";
        auto [tag, zip_url, checksum_url] = fetch_release();

        if (tag.empty() || zip_url.empty()) {
            log("Status ~ Release Fetch Failed");
            return std::filesystem::exists(kit_dir);
        }
        log(fmt::format("Release ~ {}", tag));

        auto do_download = [&]() -> bool {
            std::filesystem::remove_all(kit_dir);
            log("Status ~ Downloading");
            if (!download(zip_url, zip_path)) { log("Status ~ Download Failed"); return false; }
            log("Status ~ Extracting");
            if (!extract(zip_path, kit_dir)) { log("Status ~ Extraction Failed"); return false; }
            std::filesystem::remove(zip_path);
            version.clear();
            log(fmt::format("Status ~ Ready | Version ~ {}", get_version()));
            return true;
        };

        auto validate_files = [&](const rapidjson::Document& checksum_doc) -> bool {
            if (!checksum_doc.HasMember("files") || !checksum_doc["files"].IsObject()) { log("Checksum ~ Invalid | Reason ~ Missing files"); return false; }
            const auto& files = checksum_doc["files"];
            const int total = static_cast<int>(files.MemberCount());
            int checked = 0;
            for (auto it = files.MemberBegin(); it != files.MemberEnd(); ++it) {
                const std::string rel_path = it -> name.GetString();
                if (!it -> value.IsObject() || !it -> value.HasMember("sha256") || !it -> value["sha256"].IsString()) { log(fmt::format("Checksum ~ Malformed | File ~ {}", rel_path)); return false; }
                const std::string expected = it -> value["sha256"].GetString();
                if (!Tool::File::exists(kit_dir, rel_path)) { log(fmt::format("File ~ Missing | Progress ~ {}/{} | Path ~ {}", checked, total, rel_path)); return false; }
                if (Tool::Crypto::hash_file("SHA256", kit_dir + "/" + rel_path) != expected) { log(fmt::format("Checksum ~ Mismatch | Progress ~ {}/{} | File ~ {}", checked, total, rel_path)); return false; }
                ++checked;
            }
            log(fmt::format("Status ~ Cache Valid | Files ~ {}/{} | Version ~ {}", checked, total, get_version()));
            return true;
        };

        auto needs_download = [&]() -> bool {
            if (!std::filesystem::exists(kit_dir)) { log("Status ~ Cache Missing"); return true; }
            std::string remote_hash;
            auto checksum_doc = fetch_checksum(checksum_url, remote_hash);
            if (checksum_doc.HasParseError() || !checksum_doc.IsObject()) { log("Checksum ~ Fetch Failed"); return true; }
            if (get_version() != tag) { log(fmt::format("Version ~ Mismatch | Local ~ {} | Remote ~ {}", get_version(), tag)); return true; }
            if (!remote_hash.empty() && Tool::Crypto::hash_file("SHA256", kit_dir + "/checksum.json") != remote_hash) { log("Checksum ~ Tampered"); return true; }
            return !validate_files(checksum_doc);
        };

        return needs_download() ? do_download() : true;
    }
}