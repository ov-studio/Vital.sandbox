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


//////////////////////////
// Vital: Manager: Kit //
//////////////////////////

namespace Vital::Manager::Kit {
    const std::string toolkit_api = "https://api.github.com/repos/ov-studio/Vital.kit/releases/latest";
    const std::string cache_base  = "cache";
    const std::string kit_name    = "Vital.kit";
    std::mutex content_mutex;
    std::unordered_map<std::string, std::string> content_cache;
    std::unordered_map<std::string, rapidjson::Document> json_cache;

    const Tool::Rest::rest_headers kit_headers = { "User-Agent: Vital.sandbox" };
    static std::string s_version;

    const std::string& get_version() {
        if (!s_version.empty()) return s_version;
        const std::string checksum_path = cache_base + "/" + kit_name + "/checksum.json";
        std::ifstream f(checksum_path, std::ios::binary);
        if (!f) {
            Vital::print("sbox", "Kit: get_version -> checksum.json not found");
            return s_version;
        }
        std::string raw{ std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
        rapidjson::Document doc;
        doc.Parse(raw.c_str());
        if (doc.HasParseError() || !doc.IsObject()) {
            Vital::print("sbox", "Kit: get_version -> checksum.json parse error");
            return s_version;
        }
        if (doc.HasMember("version") && doc["version"].IsString()) s_version = doc["version"].GetString();
        return s_version;
    }

    std::tuple<std::string, std::string, std::string> fetch_release_info() {
        std::string response;
        try { response = Tool::Rest::get(toolkit_api, kit_headers); }
        catch (const std::exception& e) {
            Vital::print("sbox", "Kit: release fetch error -> ", e.what());
            return {};
        }
        if (response.empty()) {
            Vital::print("sbox", "Kit: release response empty");
            return {};
        }

        rapidjson::Document doc;
        doc.Parse(response.c_str());
        if (doc.HasParseError()) {
            Vital::print("sbox", "Kit: release JSON parse error -> code: ", std::to_string(doc.GetParseError()).c_str(), " offset: ", std::to_string(doc.GetErrorOffset()).c_str());
            Vital::print("sbox", "Kit: raw response -> ", response.substr(0, 200).c_str());
            return {};
        }
        if (!doc.IsObject()) {
            Vital::print("sbox", "Kit: release JSON not object");
            return {};
        }

        std::string tag, zip_url, checksum_url;
        if (doc.HasMember("tag_name") && doc["tag_name"].IsString())
            tag = doc["tag_name"].GetString();
        if (doc.HasMember("assets") && doc["assets"].IsArray()) {
            for (auto& asset : doc["assets"].GetArray()) {
                if (!asset.HasMember("name") || !asset.HasMember("browser_download_url")) continue;
                std::string asset_name = asset["name"].GetString();
                std::string asset_url  = asset["browser_download_url"].GetString();
                if (asset_name == "checksum.json")
                    checksum_url = asset_url;
                if (asset_name.rfind(kit_name, 0) == 0 && asset_name.size() > 4 &&
                    asset_name.substr(asset_name.size() - 4) == ".zip")
                    zip_url = asset_url;
            }
        }
        return { tag, zip_url, checksum_url };
    }

    rapidjson::Document fetch_checksum(const std::string& checksum_url, std::string& out_remote_hash) {
        rapidjson::Document doc;
        if (checksum_url.empty()) return doc;
        std::string data;
        try { data = Tool::Rest::get(checksum_url, kit_headers); }
        catch (const std::exception& e) {
            Vital::print("sbox", "Kit: checksum fetch error -> ", e.what());
            return doc;
        }
        if (data.empty()) return doc;
        out_remote_hash = Tool::Crypto::hash("SHA256", data);
        doc.Parse(data.c_str());
        return doc;
    }

    bool extract_zip(const std::string& zip_path, const std::string& dest_dir) {
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

            std::filesystem::path out = std::filesystem::path(dest_dir) / entry;
            std::filesystem::create_directories(out.parent_path());
            std::ofstream out_f(out, std::ios::binary | std::ios::trunc);
            if (out_f) out_f.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        }

        zip_close(archive);
        return true;
    }

    bool download_file(const std::string& url, const std::string& dest_path) {
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

    bool ensure_kit() {
        const std::string kit_dir  = cache_base + "/" + kit_name;
        const std::string zip_path = cache_base + "/" + kit_name + ".zip";
        auto [tag, zip_url, checksum_url] = fetch_release_info();

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
            if (!download_file(zip_url, zip_path)) {
                Vital::print("sbox", "Kit: download failed");
                return false;
            }
            Vital::print("sbox", "Kit: extracting...");
            if (!extract_zip(zip_path, kit_dir)) {
                Vital::print("sbox", "Kit: extraction failed");
                return false;
            }
            std::filesystem::remove(zip_path);
            s_version.clear();
            Vital::print("sbox", "Kit: ready ( ", get_version(), " )");
        }
        return true;
    }

    std::string fetch_file(const std::string& rel_path) {
        std::filesystem::path full = std::filesystem::path(Kit::cache_base)/Kit::kit_name/rel_path;
        std::ifstream f(full, std::ios::binary);
        if (!f) return {};
        return { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
    }

    const std::string& fetch_content(std::string_view path) {
        std::lock_guard<std::mutex> lock(content_mutex);
        std::string key(path);
        auto it = content_cache.find(key);
        if (it != content_cache.end()) return it -> second;
        std::string value = fetch_file(key);
        if (value.empty()) {
            static const std::string empty{};
            return empty;
        }
        return content_cache.emplace(key, std::move(value)).first -> second;
    }

    rapidjson::Document& fetch_json(const std::string& name) {
        auto it = json_cache.find(name);
        if (it != json_cache.end()) return it -> second;
        rapidjson::Document document;
        document.Parse(fetch_content(name + ".json").c_str());
        if (document.HasParseError()) {
            static rapidjson::Document empty_doc;
            return empty_doc;
        }
        return json_cache.emplace(name, std::move(document)).first -> second;
    }

    std::string fetch_module(const std::string& name) {
        auto& document = fetch_json(name + "/manifest");
        if (document.HasParseError() || !document.HasMember("source")) return "";
        return fetch_content(name + "/" + document["source"].GetString());
    }

    std::vector<std::string> fetch_modules(const std::string& name) {
        std::vector<std::string> result;
        auto& document = fetch_json(name + "/manifest");
        if (document.HasParseError() || !document.HasMember("sources") || !document["sources"].IsArray()) return result;
        for (auto& i : document["sources"].GetArray()) {
            result.push_back(fetch_content(name + "/" + std::string(i.GetString())));
        }
        return result;
    }
}