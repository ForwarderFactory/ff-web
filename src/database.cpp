#include <ff.hpp>
#include <scrypto.hpp>
#include <nlohmann/json.hpp>

// implement changes made to the database schema
void ff::update_to_latest(database& database) {
    // 1. now ensure that the users table, in the json column, has the json object 'profile'
    for (const auto& it : database.query("SELECT * FROM users;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (json.find("profile") == json.end()) {
            json["profile"] = nlohmann::json::object();
            database.exec("UPDATE users SET json = ? WHERE id = ?;", json.dump(), it.at("id"));
        }
    }

    // 2. ensure that the json in forwarders table has [forwarders][x][meta][vwii_compatible] and it is a boolean
    for (const auto& it : database.query("SELECT * FROM forwarders;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (json.find("meta") == json.end() || !json.at("meta").is_object()) {
            continue; // skip if meta is not an object
        }

        if (json.at("meta").find("vwii_compatible") == json.at("meta").end() || !json.at("meta").at("vwii_compatible").is_boolean()) {
            // test to see if it's vwii compatible
            // get data_download_key from the json
            const std::string data_download_key = json.at("data_download_key").get<std::string>();
            const auto ret = database.query("SELECT * FROM files WHERE file_id = ?;", data_download_key);
            if (ret.empty()) {
                continue; // skip if the file does not exist
            }
            const auto& file_json = nlohmann::json::parse(ret.at(0).at("json")).at("path").get<std::string>();
            const auto r = ff::get_info_from_wad(file_json);

            // remove because maybe if it's a different type it'll cast my rvalue?
            if (json.at("meta").find("vwii_compatible") != json.at("meta").end()) {
                json.at("meta").erase("vwii_compatible");
            }

            json["meta"]["vwii_compatible"] = r.supports_vwii;
        }

        database.exec("UPDATE forwarders SET json = ? WHERE id = ?;", json.dump(), it.at("id"));
    }

    // ensure that sandbox json has [ratings] and that it is an object
    for (const auto& it : database.query("SELECT * FROM sandbox;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (json.find("ratings") == json.end() || !json.at("ratings").is_object()) {
            json["ratings"] = nlohmann::json::object();
        }
        if (json.find("reviews") == json.end() || !json.at("reviews").is_array()) {
            json["reviews"] = nlohmann::json::array();
        }

        database.exec("UPDATE sandbox SET json = ? WHERE id = ?;", json.dump(), it.at("id"));
    }
    // same for forwarders
    for (const auto& it : database.query("SELECT * FROM forwarders;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (json.find("ratings") == json.end() || !json.at("ratings").is_object()) {
            json["ratings"] = nlohmann::json::object();
        }
        if (json.find("reviews") == json.end() || !json.at("reviews").is_array()) {
            json["reviews"] = nlohmann::json::array();
        }

        database.exec("UPDATE forwarders SET json = ? WHERE id = ?;", json.dump(), it.at("id"));
    }

    // if there is no thumbnail, generate one
    for (const auto& it : database.query("SELECT * FROM forwarders;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json db_json;
        try {
            db_json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (db_json.find("banner_thumbnail_download_key") == db_json.end() ||
            db_json.at("banner_thumbnail_download_key").is_string() == false ||
            db_json.at("banner_thumbnail_download_key").get<std::string>().empty()) {

            // generate a thumbnail
            // if the banner is a video, generate a thumbnail
            // otherwise use the banner as the thumbnail
            const std::string banner_type = db_json.at("meta").at("banner_type");
            const std::string banner_key = db_json.at("banner_download_key").get<std::string>();

            if (banner_type == "video") {
                // get the file path from the database
                const auto ret = database.query("SELECT * FROM files WHERE file_id = ?;", banner_key);

                if (ret.empty()) {
                    continue; // skip if the file does not exist
                }

                const std::string banner_path = nlohmann::json::parse(ret.at(0).at("json")).at("path").get<std::string>();

                std::string thumbnail_path = ff::get_temp_path() + "/thumbnail.webp";
                if (!ff::generate_thumbnail(banner_path, thumbnail_path)) {
                    throw std::runtime_error{"Failed to generate thumbnail for banner."};
                }
                std::string thumbnail_key = ff::upload_file(database, FileConstruct{
                    .path = thumbnail_path,
                    .name = "banner_thumbnail.webp",
                });
                if (thumbnail_key.empty()) {
                    throw std::runtime_error{"Failed to upload thumbnail for banner."};
                }

                db_json["banner_thumbnail_download_key"] = thumbnail_key;
                db_json["banner_thumbnail_type"] = "image";
            } else {
                // simply copy the banner as the thumbnail
                db_json["banner_thumbnail_download_key"] = banner_key;
                db_json["banner_thumbnail_type"] = "image";
            }
        }

        if (db_json.find("icon_thumbnail_download_key") == db_json.end() ||
            db_json.at("icon_thumbnail_download_key").is_string() == false ||
            db_json.at("icon_thumbnail_download_key").get<std::string>().empty()) {

            // generate a thumbnail
            // if the icon is a video, generate a thumbnail
            // otherwise use the icon as the thumbnail
            const std::string icon_type = db_json.at("meta").at("icon_type");
            const std::string icon_key = db_json.at("icon_download_key").get<std::string>();

            if (icon_type == "video") {
                // get the file path from the database
                const auto ret = database.query("SELECT * FROM files WHERE file_id = ?;", icon_key);

                if (ret.empty()) {
                    continue; // skip if the file does not exist
                }

                const std::string icon_path = nlohmann::json::parse(ret.at(0).at("json")).at("path").get<std::string>();

                std::string thumbnail_path = ff::get_temp_path() + "/thumbnail.webp";
                if (!ff::generate_thumbnail(icon_path, thumbnail_path)) {
                    throw std::runtime_error{"Failed to generate thumbnail for icon."};
                }
                std::string thumbnail_key = ff::upload_file(database, FileConstruct{
                    .path = thumbnail_path,
                    .name = "icon_thumbnail.webp",
                });
                if (thumbnail_key.empty()) {
                    throw std::runtime_error{"Failed to upload thumbnail for icon."};
                }

                db_json["icon_thumbnail_download_key"] = thumbnail_key;
                db_json["icon_thumbnail_type"] = "image";
            } else {
                // simply copy the icon as the thumbnail
                db_json["icon_thumbnail_download_key"] = icon_key;
                db_json["icon_thumbnail_type"] = "image";
            }
        }
    }

    // in sandbox, replace data_download_key with [data][x][download_key] and -||-filename
    for (const auto& it : database.query("SELECT * FROM sandbox;")) {
        if (it.empty()) {
            continue;
        }

        nlohmann::json db_json;
        try {
            db_json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            continue; // skip if the json is invalid
        }

        if (db_json.find("data") == db_json.end() || !db_json.at("data").is_array()) {
            db_json["data"] = nlohmann::json::array();
            db_json["filenames"] = nlohmann::json::array();

            if (db_json.find("data_download_key") == db_json.end() ||
                !db_json.at("data_download_key").is_string() ||
                db_json.at("data_download_key").get<std::string>().empty()) {
                throw std::runtime_error{"data_download_key is missing in sandbox json."};
            }

            const std::string& data_download_key = db_json.at("data_download_key").get<std::string>();
            const auto ret = database.query("SELECT * FROM files WHERE file_id = ?;", data_download_key);
            if (ret.empty()) {
                throw std::runtime_error{"File with data_download_key does not exist."};
            }
            const std::string file_path = nlohmann::json::parse(ret.at(0).at("json")).at("path").get<std::string>();
            const std::string file_name = nlohmann::json::parse(ret.at(0).at("json")).at("filename").get<std::string>();
            db_json["data"].push_back({
                {"download_key", data_download_key},
                {"filename", file_name}
            });
            db_json["filenames"].push_back(file_name);

            logger.write_to_log(limhamn::logger::type::notice, "Updated sandbox json with data_download_key: " + data_download_key + " and filename: " + file_name + "\n");

            // write to the database
            database.exec("UPDATE sandbox SET json = ? WHERE id = ?;", db_json.dump(), it.at("id"));
        }
    }
}

void ff::setup_database(database& database) {
    std::string primary = "id INTEGER PRIMARY KEY";
    if (ff::settings.enabled_database) {
        primary = "id SERIAL PRIMARY KEY";
    }

    // id: the user id
    // username: the username of the user
    // password: the password of the user
    // key: the key, stored in the database and in the user's cookie, used to authenticate the user
    // email: the email of the user
    // created_at: the time the user was created
    // updated_at: the time the user was last updated
    // ip_address: the ip address of the user
    // user_agent: the user agent of the user
    // user_type: 0 = User, 1 = Administrator
    // json: the json of the user
    if (!database.exec("CREATE TABLE IF NOT EXISTS users (" + primary + ", username TEXT NOT NULL, password TEXT NOT NULL, key TEXT NOT NULL, email TEXT NOT NULL, created_at bigint NOT NULL, updated_at bigint NOT NULL, ip_address TEXT NOT NULL, user_agent TEXT NOT NULL, user_type bigint NOT NULL, json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the users table."};
    }

    // id: the url id
    // url: the activation url
    // created_at: the time the url was created
    // username: the username of the user
    if (!database.exec("CREATE TABLE IF NOT EXISTS activation_urls (" + primary + ", url TEXT NOT NULL, created_at bigint NOT NULL, username TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the activation_urls table."};
    }

    // id: the forwarder id
    // identifier: the page identifier
    // json: the json of the forwarder
    if (!database.exec("CREATE TABLE IF NOT EXISTS forwarders (" + primary + ", identifier TEXT NOT NULL, json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the forwarders table."};
    }

    // id: the forwarder id
    // identifier: the page identifier
    // json: the json of the forwarder
    if (!database.exec("CREATE TABLE IF NOT EXISTS sandbox (" + primary + ", identifier TEXT NOT NULL, json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the sandbox table."};
    }

    // id: the file id
    // file_id: identifier of the file, used to retrieve the file
    // json: the json of the file (including actual path)
    if (!database.exec("CREATE TABLE IF NOT EXISTS files (" + primary + ", file_id TEXT NOT NULL, json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the files table."};
    }

    // id: the general id
    // json: the json of the general settings
    if (!database.exec("CREATE TABLE IF NOT EXISTS general (" + primary + ", json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the general table."};
    }

    const auto query = database.query("SELECT * FROM general;");
    if (query.empty()) {
        nlohmann::json json;
        json["announcements"] = nlohmann::json::array();

        database.exec("INSERT INTO general (json) VALUES (?);", json.dump());
    }

    try
    {
        update_to_latest(database);
    } catch (const std::exception& e) {
        throw std::runtime_error{"Error updating database to latest version: " + std::string{e.what()}};
    }
}

std::string ff::upload_file(database& db, const ff::FileConstruct& c) {
    if (!db.good()) {
        throw std::runtime_error{"Database is not good."};
    }
    if (c.path.empty() || c.name.empty()) {
        throw std::runtime_error{"File or name is empty."};
    }
    if (!std::filesystem::is_regular_file(c.path)) {
        throw std::runtime_error{"File is not a regular file."};
    }

    nlohmann::json json;

    json["filename"] = c.name;
    json["username"] = c.username;
    json["ip_address"] = c.ip_address;
    json["user_agent"] = c.user_agent;
    json["uploaded_at"] = scrypto::return_unix_timestamp();
    json["downloads"] = 0;
    json["downloaders"] = nlohmann::json::array(); /* combine username, ip address, user agent and timestamp */

    const auto check_for_dup = [](database& db, const std::string& key) -> bool {
        for (const auto& it : db.query("SELECT * FROM files WHERE file_id = ?;", key)) {
            if (it.empty()) {
                return false;
            }

            return true;
        }

        return false;
    };

    // generate a random key
    std::string key = scrypto::generate_random_string(16);
    std::string file_key = scrypto::generate_random_string(16);

    int i{0};
    while (check_for_dup(db, key) || i < 10) {
        key = scrypto::generate_random_string(16);
        i++; // prevent infinite loop that could potentially occur if the RNG is bad
    }

    i = 0;
    while (check_for_dup(db, file_key) || i < 10) {
        file_key = scrypto::generate_random_string(16);
        i++;
    }

    // create directory if it doesn't exist
    std::filesystem::path dir{ff::settings.data_directory + "/" + key};
    if (!std::filesystem::is_directory(dir)) {
        std::filesystem::create_directories(dir);
    }

    dir += "/" + file_key;
    std::filesystem::copy_file(c.path, dir);
    if (!std::filesystem::is_regular_file(dir)) {
        throw std::runtime_error{"Failed to move file."};
    }

    json["path"] = dir;
    json["size"] = std::filesystem::file_size(dir);

    if (std::filesystem::file_size(dir) <= ff::settings.max_file_size_hash) {
        json["sha256"] = scrypto::sha256hash_file(dir);
    }

#if FF_DEBUG
    // assert that the file exists and that it's identical to the original file
    if (!std::filesystem::exists(dir)) {
        throw std::runtime_error{"File does not exist after upload."};
    }
    if (std::filesystem::file_size(dir) != std::filesystem::file_size(c.path)) {
        throw std::runtime_error{"File size does not match after upload."};
    }
#endif

    // insert into the files table
    if (!db.exec("INSERT INTO files (file_id, json) VALUES (?, ?);", file_key, json.dump())) {
        throw std::runtime_error{"Error inserting into the files table."};
    }

    return file_key;
}

std::string ff::get_path_from_file(database& db, const std::string& file_key) {
    if (!db.good()) {
        throw std::runtime_error{"Database is not good."};
    }
    if (file_key.empty()) {
        throw std::runtime_error{"File key is empty."};
    }

    for (const auto& it : db.query("SELECT * FROM files WHERE file_id = ?;", file_key)) {
        if (it.empty()) {
            throw std::runtime_error{"File not found."};
        }

        nlohmann::json json;
        try {
            json = nlohmann::json::parse(it.at("json"));
        } catch (const std::exception&) {
            throw std::runtime_error{"Error parsing JSON."};
        }

        if (json.find("path") == json.end() || !json.at("path").is_string()) {
            throw std::runtime_error{"Path not found in JSON."};
        }

        return json.at("path").get<std::string>();
    }

    throw std::runtime_error{"File not found."};
}

bool ff::is_file(database& db, const std::string& file_key) {
    if (!db.good()) {
        throw std::runtime_error{"Database is not good."};
    }
    if (file_key.empty()) {
        throw std::runtime_error{"File key is empty."};
    }

    for (const auto& it : db.query("SELECT * FROM files WHERE file_id = ?;", file_key)) {
        if (it.empty()) {
            return false;
        }

        return true;
    }

    return false;

}

ff::RetrievedFile ff::download_file(database& db, const ff::UserProperties& prop, const std::string& file_key) {
    if (!db.good()) {
        throw std::runtime_error{"Database is not good."};
    }
    if (prop.ip_address.empty() || prop.user_agent.empty() || file_key.empty()) {
        throw std::runtime_error{"IP address, user agent, or file key is empty."};
    }

    nlohmann::json downloaders;
    downloaders["username"] = prop.username;
    downloaders["ip_address"] = prop.ip_address;
    downloaders["user_agent"] = prop.user_agent;
    downloaders["timestamp"] = scrypto::return_unix_timestamp();

    if (prop.username.empty()) {
        downloaders["username"] = "_nouser_";
    }

    /* select all matching file_key */
    const auto query = db.query("SELECT * FROM files WHERE file_id = ?;", file_key);
    if (query.empty()) {
        throw std::runtime_error{"Query is empty."};
    }

    // get json
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(query.at(0).at("json"));
    } catch (const std::exception&) {
        throw std::runtime_error{"Error parsing JSON."};
    }

    if (json.find("downloads") != json.end() && json.at("downloads").is_number()) {
        json["downloads"] = json.at("downloads").get<int>() + 1;
    }
    if (json.find("downloaders") != json.end() && json.at("downloaders").is_array()) {
        json["downloaders"].push_back(downloaders);
    }
    if (json.find("filename") == json.end() || !json.at("filename").is_string()) {
        throw std::runtime_error{"Filename not found."};
    }

    ff::RetrievedFile f;
    if (json.find("path") == json.end() || !json.at("path").is_string()) {
        throw std::runtime_error{"Path not found."};
    }
    if (!std::filesystem::is_regular_file(json.at("path").get<std::string>())) {
        throw std::runtime_error{"File is not a regular file: " + json.at("path").get<std::string>()};
    }

    f.name = json.at("filename").get<std::string>();
    f.path = json.at("path").get<std::string>();

    // reinsert into the files table
    if (!db.exec("UPDATE files SET json = ? WHERE file_id = ?;", json.dump(), file_key)) {
        throw std::runtime_error{"Error updating the files table."};
    }

    return f;
}

std::string ff::get_json_from_table(database& db, const std::string& table, const std::string& key, const std::string& value) {
    if (!db.good()) {
        throw std::runtime_error{"Database is not good."};
    }
    if (table.empty() || key.empty() || value.empty()) {
        throw std::runtime_error{"Table, key, or value is empty."};
    }

    const auto& query = db.query("SELECT json FROM " + table + " WHERE " + key + " = ?;", value);
    if (query.empty()) {
        throw std::runtime_error{"Query is empty."};
    }

    for (const auto& it : query) {
        if (it.find("json") == it.end()) {
            throw std::runtime_error{"JSON not found."};
        }

        return it.at("json");
    }

    throw std::runtime_error{"JSON not found."};
}

bool ff::set_json_in_table(database& db, const std::string& table, const std::string& key, const std::string& value, const std::string& json) {
    if (!db.good() || table.empty() || key.empty() || value.empty() || json.empty()) {
        return false;
    }

    return db.exec("UPDATE " + table + " SET json = ? WHERE " + key + " = ?;", json, value);
}
