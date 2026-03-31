/*----------------------------------------------------------------
     Resource: Vital.sandbox
     Script: Tool: module.cpp
     Author: ov-studio
     Developer(s): Aviril, Tron, Mario, Аниса, A-Variakojiene
     DOC: 14/09/2022
     Desc: Module Tools
----------------------------------------------------------------*/


//////////////
// Imports //
//////////////

#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <Vital.sandbox/Tool/module.h>
#include <Vital.extension/Engine/public/console.h>


//////////////////////////
// Vital: Tool: Module //
//////////////////////////

namespace Vital::Tool {
    // TODO: Improve
    std::mutex content_mutex;
    std::unordered_map<std::string, std::string> content_cache;
    std::unordered_map<std::string, rapidjson::Document> json_cache;
    const std::string toolkit_api = "https://api.github.com/repos/ov-studio/Vital.kit/releases/latest";
    const std::string cache_base  = "cache";
    const std::string kit_name = "Vital.kit";

    std::string read_kit_file(const std::string& rel_path) {
        std::filesystem::path full = std::filesystem::path(cache_base) / kit_name / rel_path;
        std::ifstream f(full, std::ios::binary);
        if (!f) return {};
        return { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
    }

    const std::string& fetch_content(std::string_view path) {
        std::lock_guard<std::mutex> lock(content_mutex);
        std::string key(path);
        auto it = content_cache.find(key);
        if (it != content_cache.end()) return it -> second;
        std::string value = read_kit_file(key);
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


    ///////////////////////////
    // Kit: Release & Cache //
    ///////////////////////////

    namespace Kit {
        const Rest::rest_headers kit_headers = { "User-Agent: Vital.extension" };
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
            try { response = Rest::get(toolkit_api, kit_headers); }
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

        rapidjson::Document fetch_checksum(const std::string& checksum_url) {
            rapidjson::Document doc;
            if (checksum_url.empty()) return doc;
            std::string data;
            try { data = Rest::get(checksum_url, kit_headers); }
            catch (const std::exception& e) {
                Vital::print("sbox", "Kit: checksum fetch error -> ", e.what());
                return doc;
            }
            if (data.empty()) return doc;
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
            try { data = Rest::get(url, kit_headers, 120); }
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
            const std::string kit_dir = cache_base + "/" + kit_name;
            const std::string zip_path = cache_base + "/" + kit_name + ".zip";
            const std::string local_checksum = kit_dir + "/checksum.json";

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
            else if (!std::filesystem::exists(local_checksum)) {
                Vital::print("sbox", "Kit: checksum.json missing from kit — will redownload");
                needs_download = true;
            }
            else {
                auto checksum_doc = fetch_checksum(checksum_url);
                if (checksum_doc.HasParseError()) {
                    Vital::print("sbox", "Kit: checksum parse error — will redownload");
                    needs_download = true;
                }
                else if (!checksum_doc.IsObject()) {
                    Vital::print("sbox", "Kit: checksum not an object — will redownload");
                    needs_download = true;
                }
                else if (!checksum_doc.HasMember("files") || !checksum_doc["files"].IsObject()) {
                    Vital::print("sbox", "Kit: checksum missing/invalid 'files' — will redownload");
                    needs_download = true;
                }
                else {
                    const auto& files = checksum_doc["files"];
                    bool all_valid = true;
                    for (auto it = files.MemberBegin(); it != files.MemberEnd(); ++it) {
                        const std::string rel_path = it -> name.GetString();
                        if (!it -> value.IsObject() || !it -> value.HasMember("sha256") || !it -> value["sha256"].IsString()) {
                            Vital::print("sbox", "Kit: checksum entry malformed -> ", rel_path.c_str());
                            all_valid = false;
                            break;
                        }
                        const std::string expected = it -> value["sha256"].GetString();
                        if (!Vital::Tool::File::exists(kit_dir, rel_path)) {
                            Vital::print("sbox", "Kit: file missing -> ", rel_path.c_str());
                            all_valid = false;
                            break;
                        }
                        const std::string actual = Vital::Tool::Crypto::hash_file("SHA256", kit_dir + "/" + rel_path);
                        if (actual != expected) {
                            Vital::print("sbox", "Kit: checksum mismatch -> ", rel_path.c_str(), " expected: ", expected.c_str(), " got: ", actual.c_str());
                            all_valid = false;
                            break;
                        }
                    }
                    if (!all_valid) needs_download = true;
                }
            }

            if (!needs_download) {
                Vital::print("sbox", "Kit: cache valid — skipping download (", get_version(), ")");
                return true;
            }
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
            s_version_resolved = false;
            Vital::print("sbox", "Kit: ready (", get_version(), ")");
            return true;
        }
    }
}