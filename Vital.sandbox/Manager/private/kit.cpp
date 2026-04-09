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
    bool download(const std::string& url, const std::string& dest_path) {
        std::string data;
        try { data = Tool::Rest::get(url, kit_headers, 120); }
        catch (const std::exception& e) {
            Vital::print("sbox", "Kit: download error -> ", e.what());
            return false;
        }
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
            while ((n = zip_fread(zf, buf, sizeof(buf))) > 0) {
                contents.append(buf, static_cast<size_t>(n));                
            }
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
            Vital::print("sbox", "Kit: failed to fetch release info");
            return std::filesystem::exists(kit_dir);
        }
        Vital::print("sbox", "Kit: release -> ", tag.c_str());

        bool needs_download = false;
        if (!std::filesystem::exists(kit_dir)) {
            Vital::print("sbox", "Kit: cache missing — will download");
            needs_download = true;
        }
        else {
            std::string remote_hash;
            auto checksum_doc = fetch_checksum(checksum_url, remote_hash);
            if (checksum_doc.HasParseError() || !checksum_doc.IsObject()) {
                Vital::print("sbox", "Kit: checksum fetch failed — will redownload");
                needs_download = true;
            }
            else if (get_version() != tag) {
                Vital::print("sbox", fmt::format("Kit: version mismatch — local ( {} ) remote ( {} ) — will redownload", get_version(), tag).c_str());
                needs_download = true;
            }
            else {
                if (!remote_hash.empty()) {
                    const std::string local_hash = Tool::Crypto::hash_file("SHA256", kit_dir + "/checksum.json");
                    if (local_hash != remote_hash) {
                        Vital::print("sbox", "Kit: checksum tampered — will redownload");
                        needs_download = true;
                    }
                }
                if (!needs_download) {
                    if (!checksum_doc.HasMember("files") || !checksum_doc["files"].IsObject()) {
                        Vital::print("sbox", "Kit: checksum missing/invalid 'files' — will redownload");
                        needs_download = true;
                    }
                    else {
                        const auto& files = checksum_doc["files"];
                        const int total = static_cast<int>(files.MemberCount());
                        int checked = 0;
                        bool all_valid = true;
                        for (auto it = files.MemberBegin(); it != files.MemberEnd(); ++it) {
                            const std::string rel_path = it -> name.GetString();
                            if (!it -> value.IsObject() || !it -> value.HasMember("sha256") || !it -> value["sha256"].IsString()) {
                                Vital::print("sbox", "Kit: checksum entry malformed -> ", rel_path.c_str());
                                all_valid = false;
                                break;
                            }
                            const std::string expected = it -> value["sha256"].GetString();
                            if (!Tool::File::exists(kit_dir, rel_path)) {
                                Vital::print("sbox", fmt::format("Kit: file missing ({}/{}) -> {}", checked, total, rel_path).c_str());
                                all_valid = false;
                                break;
                            }
                            const std::string actual = Tool::Crypto::hash_file("SHA256", kit_dir + "/" + rel_path);
                            if (actual != expected) {
                                Vital::print("sbox", fmt::format("Kit: checksum mismatch ({}/{}) -> {}", checked, total, rel_path).c_str());
                                all_valid = false;
                                break;
                            }
                            ++checked;
                        }
                        if (!all_valid) needs_download = true;
                        else Vital::print("sbox", fmt::format("Kit: cache valid ({}/{} files) — skipping download ( {} )", checked, total, get_version()).c_str());
                    }
                }
            }
        }

        if (needs_download) {
            std::filesystem::remove_all(kit_dir);
            Vital::print("sbox", "Kit: downloading...");
            if (!download(zip_url, zip_path)) {
                Vital::print("sbox", "Kit: download failed");
                return false;
            }
            Vital::print("sbox", "Kit: extracting...");
            if (!extract(zip_path, kit_dir)) {
                Vital::print("sbox", "Kit: extraction failed");
                return false;
            }
            std::filesystem::remove(zip_path);
            version.clear();
            Vital::print("sbox", "Kit: ready ( ", get_version(), " )");
        }
        return true;
    }
}