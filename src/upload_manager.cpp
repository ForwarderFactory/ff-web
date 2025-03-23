#include <filesystem>
#include <scrypto.hpp>
#include <ff.hpp>
#include <nlohmann/json.hpp>
#include <limhamn/http/http_utils.hpp>

/*
 * Warning: This function performs absolutely no credential checks. That should be performed
 * before calling this function.
 */
std::pair<ff::UploadStatus, std::string> ff::try_upload(const limhamn::http::server::request& req, database& db) {

    std::string json{};
    std::string banner_path{};
    std::string icon_path{};
    std::string wad_path{};
    std::string username{};
    bool auth{false};

    if (username_is_stored(req)) { // is session cookie
        username = req.session.at("username");
        const std::string key = req.session.at("key");

        if (!verify_key(db, username, key)) {
            return {ff::UploadStatus::InvalidCreds, ""};
        }
        auth = true;
    }

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to upload a file.\n");
#endif

    const auto file_handles = limhamn::http::utils::parse_multipart_form_file(req.raw_body, settings.temp_directory + "/%f-%h-%r");
    for (const auto& it : file_handles) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "File name: " + it.filename + ", Name: " + it.name + "\n");
#endif
        if (it.name == "json") {
            json = ff::open_file(it.path);
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got JSON\n");
#endif
        } else if (it.name == "banner") {
            banner_path = it.path;
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got banner\n");
#endif
        } else if (it.name == "icon") {
            icon_path = it.path;
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got icon\n");
#endif
        } else if (it.name == "wad") {
            wad_path = it.path;
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got WAD\n");
#endif
        }
    }

    if (wad_path.empty() || json.empty() || banner_path.empty() || icon_path.empty()) {
        return {ff::UploadStatus::Failure, ""};
    }

    nlohmann::json user_json;
    nlohmann::json db_json;
    try {
        user_json = nlohmann::json::parse(json);
    } catch (const std::exception&) {
        return {ff::UploadStatus::Failure, ""};
    }

    if (!auth) {
        if (user_json.find("username") == user_json.end() || !user_json.at("username").is_string()) {
            return {ff::UploadStatus::InvalidCreds, ""};
        }
        if (user_json.find("key") == user_json.end() || !user_json.at("key").is_string()) {
            return {ff::UploadStatus::InvalidCreds, ""};
        }

        username = user_json.at("username").get<std::string>();
        const std::string key = user_json.at("key").get<std::string>();

        if (!verify_key(db, username, key)) {
            return {ff::UploadStatus::InvalidCreds, ""};
        }
    }

    db_json["uploader"] = username;
    db_json["ratings"] = nlohmann::json::array();
    db_json["reviews"] = nlohmann::json::array();
    db_json["submitted"] = scrypto::return_unix_timestamp();
    db_json["downloads"] = 0;
    db_json["needs_review"] = get_user_type(db, username) != UserType::Administrator;
    db_json["meta"] = nlohmann::json::object();
    db_json["meta"]["categories"] = nlohmann::json::array();

    if (user_json.find("meta") == user_json.end()) {
        return {ff::UploadStatus::Failure, ""};
    }
    if (user_json.is_object() == false) {
        return {ff::UploadStatus::Failure, ""};
    }

    nlohmann::json meta = user_json.at("meta");

    if (meta.find("description") != meta.end() && meta.at("description").is_string()) {
        if (meta.at("description").size() > 1000) {
            return {ff::UploadStatus::Failure, ""};
        }

        db_json["meta"]["description"] = limhamn::http::utils::htmlspecialchars(meta.at("description"));
    }
    if (meta.find("title_id") != meta.end() && meta.at("title_id").is_string()) {
        // require 4 characters, uppercase and A-Z and/or 0-9
        std::string title_id = meta.at("title_id");
        if (title_id.size() != 4) {
            return {ff::UploadStatus::Failure, ""};
        }
        for (const auto& c : title_id) {
            if (!std::isalnum(c) || !std::isupper(c)) {
                return {ff::UploadStatus::Failure, ""};
            }
        }
        db_json["meta"]["title_id"] = title_id;
    }
    if (meta.find("title") != meta.end() && meta.at("title").is_string()) {
        if (meta.at("title").size() > 255) {
            return {ff::UploadStatus::Failure, ""};
        }
        db_json["meta"]["title"] = limhamn::http::utils::htmlspecialchars(meta.at("title"));
    }
    if (meta.find("author") != meta.end() && meta.at("author").is_string()) {
        db_json["meta"]["author"] = limhamn::http::utils::htmlspecialchars(meta.at("author"));
    }
    if (meta.find("youtube") != meta.end() && meta.at("youtube").is_string()) {
        std::string youtube = meta.at("youtube");
        if (youtube.find("youtube.com/watch?v=") == std::string::npos && !youtube.empty()) {
            return {ff::UploadStatus::Failure, ""};
        } else if (!youtube.empty()) {
            std::size_t pos = youtube.find("v=");
            youtube = youtube.substr(pos + 2);

            pos = youtube.find('&');
            if (pos != std::string::npos) {
                youtube = youtube.substr(0, pos);
            }

            db_json["meta"]["youtube"] = limhamn::http::utils::htmlspecialchars(youtube);
        }
    }
    if (meta.find("type") != meta.end() && meta.at("type").is_string()) {
        std::string type = meta.at("type");
        if (type != "Select" && type != "Forwarder" && type != "Channel") {
            return {ff::UploadStatus::Failure, ""};
        }

        if (type == "Select" || type == "Channel") {
            db_json["meta"]["type"] = 1; // 1 = channel
        } else {
            db_json["meta"]["type"] = 0; // 0 = forwarder
        }
    } else if (meta.find("type") != meta.end() && meta.at("type").is_number_integer()) {
        int type = meta.at("type");
        if (type != 0 && type != 1) {
            return {ff::UploadStatus::Failure, ""};
        }

        db_json["meta"]["type"] = type;
    }
    if (meta.find("categories") != meta.end() && meta.at("categories").is_array()) {
        nlohmann::json categories = meta.at("categories");
        for (auto& category : categories) {
            if (category.is_string()) {
                if (category.size() > 100) {
                    return {ff::UploadStatus::Failure, ""};
                }
                db_json["meta"]["categories"].push_back(limhamn::http::utils::htmlspecialchars(category));
            }
        }
   }
    if (meta.find("location") != meta.end() && meta.at("location").is_string()) {
        if (meta.at("location").size() > 100) {
            return {ff::UploadStatus::Failure, ""};
        }
        db_json["meta"]["location"] = limhamn::http::utils::htmlspecialchars(meta.at("location"));
    }
    if (meta.find("vwii_compatible") != meta.end() && meta.at("vwii_compatible").is_string()) {
        std::string vwii_compatible = meta.at("vwii_compatible");
        if (vwii_compatible != "Select" && vwii_compatible != "Yes" && vwii_compatible != "No" && vwii_compatible != "Unknown") {
            return {ff::UploadStatus::Failure, ""};
        }

        if (vwii_compatible == "Yes") {
            db_json["meta"]["vwii_compatible"] = 1;
        } else if (vwii_compatible == "No") {
            db_json["meta"]["vwii_compatible"] = 2;
        } else {
            db_json["meta"]["vwii_compatible"] = 0;
        }
    } else if (meta.find("vwii_compatible") != meta.end() && meta.at("vwii_compatible").is_number_integer()) {
        int vwii_compatible = meta.at("vwii_compatible");
        if (vwii_compatible != 0 && vwii_compatible != 1 && vwii_compatible != 2) {
            return {ff::UploadStatus::Failure, ""};
        }

        db_json["meta"]["vwii_compatible"] = vwii_compatible;
    }

    std::string banner_ext{};
    std::string icon_ext{};
    if (ff::validate_image(banner_path)) {
        db_json["meta"]["banner_type"] = "image";
        if (settings.convert_images_to_webp) {
            if (!ff::convert_to_webp(banner_path, banner_path)) {
                return {ff::UploadStatus::Failure, ""};
            }
            banner_ext = ".webp";
        }
    } else if (ff::validate_video(banner_path)) {
        db_json["meta"]["banner_type"] = "video";
        if (settings.convert_videos_to_webm) {
            const std::string out_p = ff::get_temp_path();
            if (!ff::convert_to_webm(banner_path, out_p)) {
                return {ff::UploadStatus::Failure, ""};
            }
            std::filesystem::remove(banner_path);
            std::filesystem::rename(out_p, banner_path);
            banner_ext = ".webm";
        }
    } else {
        return {ff::UploadStatus::Failure, ""};
    }

    if (ff::validate_image(icon_path)) {
        db_json["meta"]["icon_type"] = "image";
        if (settings.convert_images_to_webp) {
            if (!ff::convert_to_webp(icon_path, icon_path)) {
                return {ff::UploadStatus::Failure, ""};
            }
            icon_ext = ".webp";
        }
    } else if (ff::validate_video(icon_path)) {
        db_json["meta"]["icon_type"] = "video";
        if (settings.convert_videos_to_webm) {
            const std::string out_p = ff::get_temp_path();
            if (!ff::convert_to_webm(icon_path, out_p)) {
                return {ff::UploadStatus::Failure, ""};
            }
            std::filesystem::remove(icon_path);
            std::filesystem::rename(out_p, icon_path);
            icon_ext = ".webm";
        }
    } else {
        return {ff::UploadStatus::Failure, ""};
    }

    std::string banner_key = ff::upload_file(db, FileConstruct{
        .path = banner_path,
        .name = "banner" + banner_ext,
        .username = username,
        .ip_address = req.ip_address,
        .user_agent = req.user_agent,
    });
    std::string icon_key = ff::upload_file(db, FileConstruct{
        .path = icon_path,
        .name = "icon" + icon_ext,
        .username = username,
        .ip_address = req.ip_address,
        .user_agent = req.user_agent,
    });

    std::string data_name;
    if (db_json["meta"].find("title") != db_json["meta"].end()) {
        data_name = db_json["meta"]["title"].get<std::string>();
        if (db_json["meta"].find("title_id") != db_json["meta"].end()) {
            data_name += "-" + db_json["meta"]["title_id"].get<std::string>();
        }
    } else {
        data_name = scrypto::generate_random_string(8);
    }

    data_name += ".wad";

    // ensure that the data_name is a valid file name
    for (auto& c : data_name) {
        if (!std::isalnum(c) && c != '-' && c != '_' && c != '.') {
            c = '_';
        }
    }

    std::string data_key = ff::upload_file(db, FileConstruct{
        .path = wad_path,
        .name = data_name,
        .username = username,
        .ip_address = req.ip_address,
        .user_agent = req.user_agent,
    });

    if (banner_key.empty()) {
        return {ff::UploadStatus::Failure, ""};
    }
    if (icon_key.empty()) {
        return {ff::UploadStatus::Failure, ""};
    }
    if (data_key.empty()) {
        return {ff::UploadStatus::Failure, ""};
    }

    db_json["banner_download_key"] = banner_key;
    db_json["icon_download_key"] = icon_key;
    db_json["data_download_key"] = data_key;

    std::string page_identifier = scrypto::generate_random_string(8);

    db_json["page_identifier"] = page_identifier;

    while (db.query("SELECT * FROM forwarders WHERE identifier = ?;", page_identifier).size() > 0) {
        page_identifier = scrypto::generate_random_string(8);
    }
    if (!db.exec("INSERT INTO forwarders (identifier, json) VALUES (?, ?);", page_identifier, db_json.dump())) {
        return {ff::UploadStatus::Failure, ""};
    }

    return {ff::UploadStatus::Success, page_identifier};
}