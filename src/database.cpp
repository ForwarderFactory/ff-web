#include <ff.hpp>
#include <scrypto.hpp>
#include <nlohmann/json.hpp>

void ff::setup_database(Database& database) {
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

    // id: the file id
    // file_id: identifier of the file, used to retrieve the file
    // json: the json of the file (including actual path)
    if (!database.exec("CREATE TABLE IF NOT EXISTS files (" + primary + ", file_id TEXT NOT NULL, json TEXT NOT NULL);")) {
        throw std::runtime_error{"Error creating the files table."};
    }
}

std::string ff::upload_file(Database& db, const ff::FileConstruct& c) {
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

    const auto check_for_dup = [](Database& db, const std::string& key) -> bool {
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
    std::filesystem::rename(c.path, dir);
    if (!std::filesystem::is_regular_file(dir)) {
        throw std::runtime_error{"Failed to move file."};
    }

    json["path"] = dir;
    json["size"] = std::filesystem::file_size(dir);

    if (std::filesystem::file_size(dir) <= ff::settings.max_file_size_hash) {
        json["sha256"] = scrypto::sha256hash_file(dir);
    }

    // insert into the files table
    if (!db.exec("INSERT INTO files (file_id, json) VALUES (?, ?);", file_key, json.dump())) {
        throw std::runtime_error{"Error inserting into the files table."};
    }

    return file_key;
}

bool ff::is_file(Database& db, const std::string& file_key) {
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

ff::RetrievedFile ff::download_file(Database& db, const ff::UserProperties& prop, const std::string& file_key) {
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
    } catch (const std::exception& e) {
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
        throw std::runtime_error{"File is not a regular file."};
    }

    f.name = json.at("filename").get<std::string>();
    f.path = json.at("path").get<std::string>();

    // reinsert into the files table
    if (!db.exec("UPDATE files SET json = ? WHERE file_id = ?;", json.dump(), file_key)) {
        throw std::runtime_error{"Error updating the files table."};
    }

    return f;
}

std::string ff::get_json_from_table(Database& db, const std::string& table, const std::string& key, const std::string& value) {
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

bool ff::set_json_in_table(Database& db, const std::string& table, const std::string& key, const std::string& value, const std::string& json) {
    if (!db.good() || table.empty() || key.empty() || value.empty() || json.empty()) {
        return false;
    }

    return db.exec("UPDATE " + table + " SET json = ? WHERE " + key + " = ?;", json, value);
}
