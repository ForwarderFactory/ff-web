#include <scrypto.hpp>
#include <ff.hpp>
#define LIMHAMN_SMTP_CLIENT_IMPL
#include <limhamn/smtp/smtp_client.hpp>
#include <nlohmann/json.hpp>
#include <limhamn/http/http_utils.hpp>

bool ff::username_is_stored(const limhamn::http::server::request& request) {
    return request.session.find("username") != request.session.end();
}

bool ff::ensure_admin_account_exists(database& database) {
    for (auto& it : database.query("SELECT * FROM users WHERE user_type = ?;", static_cast<int>(UserType::Administrator))) {
        return true;
    }

    return false;
}

bool ff::verify_key(database& database, const std::string& username, const std::string& key) {
    for (auto& it : database.query("SELECT * FROM users WHERE username = ? AND key = ?;", username, key)) {
        if (it.empty()) {
            return false;
        }

        return true;
    }

    return false;
}

bool ff::ensure_valid_creds(database& database, const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }

    for (auto& it : database.query("SELECT * FROM users WHERE username = ?;", username)) {
        if (it.empty()) {
            return false;
        }

        if (!scrypto::password_verify(password, it["password"])) {
            return false;
        }

        return true;
    }

    return false;
}

int ff::get_user_id(database& database, const std::string& username) {
    for (const auto& it : database.query("SELECT id FROM users WHERE username = ?;", username)) {
        if (it.empty()) {
            return -1;
        }

        return std::stoi(it.at("id"));
    }

    return -1;
}

ff::UserType ff::get_user_type(database& database, const std::string& username) {
    for (const auto& it : database.query("SELECT user_type FROM users WHERE username = ?;", username)) {
        if (it.empty()) {
            return ff::UserType::Undefined;
        }

        if (it.at("user_type") == "0") {
            return ff::UserType::User;
        } else if (it.at("user_type") == "1") {
            return ff::UserType::Administrator;
        } else {
            return ff::UserType::Undefined;
        }
    }

    return ff::UserType::Undefined;
}

bool ff::user_is_verified(database& database, const std::string& username) {
    if (settings.enable_email_verification == false) {
        return true;
    }

    for (const auto& it : database.query("SELECT * FROM users WHERE username = ?;", username)) {
        if (it.empty()) {
            return false;
        }

        if (it.at("email").empty()) {
            return false;
        }

        const auto json = get_json_from_table(database, "users", "username", username);
        nlohmann::json user_json = nlohmann::json::parse(json);

        if (user_json.find("activated") == user_json.end() || !user_json.at("activated").is_boolean()) {
            return false;
        }

        return user_json.at("activated");
    }

    return false;
}

std::string ff::get_email_from_username(database& database, const std::string& username) {
    for (const auto& it : database.query("SELECT email FROM users WHERE username = ?;", username)) {
        if (it.empty()) {
            return "";
        }

        return it.at("email");
    }

    return "";
}

std::string ff::get_username_from_email(database& database, const std::string& email) {
    for (const auto& it : database.query("SELECT username FROM users WHERE email = ?;", email)) {
        if (it.empty()) {
            return "";
        }

        return it.at("username");
    }

    return "";
}

// Warning: This function does not check credentials, nor does it check if the user already exists, or if the values are valid or even safe.
void ff::insert_into_user_table(database& database, const std::string& username, const std::string& password,
        const std::string& key, const std::string& email, const int64_t created_at, const int64_t updated_at, const std::string& ip_address,
        const std::string& user_agent, UserType user_type, const std::string& json) {

    std::string ua{};
    for (auto& c : user_agent) {
        if (c >= 32 && c < 127) {
            ua += c;
        }
    }

#if FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Inserting into the users table.\n");
    logger.write_to_log(limhamn::logger::type::notice, "Username: " + username + ", Password: " + password + ", Key: " + key + ", Email: " + email + ", Created at: " + std::to_string(created_at) + ", Updated at: " + std::to_string(updated_at) + ", IP Address: " + ip_address + ", User Agent: " + ua + ", User Type: " + std::to_string(static_cast<int>(user_type)) + ", JSON: " + json + "\n");
#endif

    if (!database.exec("INSERT INTO users (username, password, key, email, created_at, updated_at, ip_address, user_agent, user_type, json) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
                username,
                password,
                key,
                email,
                created_at,
                updated_at,
                ip_address,
                ua,
                static_cast<int>(user_type),
                json)) {
        throw std::runtime_error{"insert_into_user_table(): Error inserting into the users table."};
    }
}

std::pair<ff::LoginStatus, std::string> ff::try_login(database& database, const std::string& username, const std::string& password,
        const std::string& ip_address, const std::string& user_agent, limhamn::http::server::response& response) {
    const std::string base_username{username};
    const std::string base_password{password};
    const std::string base_ip_address{ip_address};
    const std::string base_user_agent{user_agent};

#if FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to login.\n");
    logger.write_to_log(limhamn::logger::type::notice, "Username: " + base_username + ", Password: " + base_password + "\n");
#endif

    for (const auto& it : database.query("SELECT * FROM users WHERE username = ?;", base_username)) {
        if (it.empty()) {
            return {ff::LoginStatus::InvalidUsername, {}};
        }

        if (!scrypto::password_verify(base_password, it.at("password"))) {
            return {ff::LoginStatus::InvalidPassword, {}};
        }

        const int64_t last_login{scrypto::return_unix_timestamp()};
        std::string key{scrypto::generate_key({base_password})};

        if (!database.exec("UPDATE users SET updated_at = ?, ip_address = ?, user_agent = ?, key = ? WHERE username = ?;", last_login, base_ip_address, base_user_agent, key, base_username)) {
            return {ff::LoginStatus::Failure, {}};
        }

        if (settings.enable_email_verification && !user_is_verified(database, base_username)) {
            return {ff::LoginStatus::Inactive, {}};
        }

        response.session["username"] = base_username;
        response.session["key"] = key;

        response.cookies.push_back({"username", base_username, .path = "/"});

        limhamn::http::server::cookie c;

        UserType type = ff::get_user_type(database, base_username);
        int user_type{0};

        if (type == UserType::Administrator) {
            user_type = 1;
        }

        response.cookies.push_back({"user_type", std::to_string(user_type)});

        return {ff::LoginStatus::Success, key};
    }

    return {ff::LoginStatus::InvalidUsername, {}};
}

// Warning: This function does not check credentials or anything. That should be done before calling this function.
// If such case is not taken, it may be possible to create an account with elevated privileges.
ff::AccountCreationStatus ff::make_account(database& database, const std::string& username, const std::string& password,
        const std::string& email, const std::string& ip_address, const std::string& user_agent, UserType user_type) {
    const std::string base_username{username};
    const std::string base_password{password};
    const std::string base_ip_address{ip_address};
    const std::string base_user_agent{user_agent};
    std::string base_email{email};
    const std::string hashed_password{scrypto::password_hash(password)};
    const std::string key{scrypto::generate_key({base_password})};
    const int64_t current_time{scrypto::return_unix_timestamp()};
    const int uploads{0};

    try {
        if (!user_is_verified(database, base_username) && settings.enable_email_verification) {
            database.exec("DELETE FROM users WHERE username = ?;", base_username);
            database.exec("DELETE FROM activation_urls WHERE username = ?;", base_username);
        }
    } catch (const std::exception&) {
        return ff::AccountCreationStatus::Failure;
    }

    nlohmann::json json;
    json["uploads"] = uploads;
    json["activated"] = base_email.empty();

    if (needs_setup) {
        json["activated"] = true;
    }

    for (auto& it : database.query("SELECT username FROM users WHERE username = ?;", base_username)) {
        if (!it.empty()) {
            return ff::AccountCreationStatus::UsernameExists;
        }
    }
    for (const auto& it : database.query("SELECT email FROM users WHERE email = ?;", base_email)) {
        if (!it.empty()) {
            return ff::AccountCreationStatus::EmailExists;
        }
    }

    if (base_username.empty()) {
        return ff::AccountCreationStatus::InvalidUsername;
    } else if (base_username.size() < ff::settings.username_min_length) {
        return ff::AccountCreationStatus::UsernameTooShort;
    } else if (base_username.size() > ff::settings.username_max_length) {
        return ff::AccountCreationStatus::UsernameTooLong;
    }

    for (auto& c : base_username) {
        if ((std::find(ff::settings.allowed_characters.begin(), ff::settings.allowed_characters.end(), c) == ff::settings.allowed_characters.end()) &&
                ff::settings.allow_all_characters == false) {
            return ff::AccountCreationStatus::InvalidUsername;
        }
    }

    if (base_password.empty() || base_password.size() < ff::settings.password_min_length) {
        return ff::AccountCreationStatus::PasswordTooShort;
    } else if (base_password.size() > ff::settings.password_max_length) {
        return ff::AccountCreationStatus::PasswordTooLong;
    }

    ff::insert_into_user_table(
            database,
            base_username,
            hashed_password,
            key,
            base_email,
            current_time,
            current_time,
            base_ip_address,
            base_user_agent,
            user_type,
            json.dump()
    );

    if (ff::settings.enable_email_verification && !needs_setup) {
        try {
            int64_t ct = scrypto::return_unix_timestamp();
            std::string activation_url = "/activate/" + scrypto::generate_random_string(32);
            if (!database.exec("INSERT INTO activation_urls (url, created_at, username) VALUES (?, ?, ?);", activation_url, ct, base_username)) {
                throw std::runtime_error{"Error inserting into the activation_urls table."};
            }

            limhamn::smtp::client::mail_properties mail_properties;

            mail_properties.from = ff::settings.email_from;
            mail_properties.to = base_email;
            mail_properties.subject = "Please activate your newly registered account.";
            mail_properties.data = "Please click the following link to activate your account: <a href=\"" + ff::settings.site_url + activation_url + "\">" + ff::settings.site_url + activation_url + "</a>";
            mail_properties.content_type = "text/html";
            mail_properties.username = ff::settings.email_username;
            mail_properties.password = ff::settings.email_password;
            mail_properties.smtp_server = ff::settings.smtp_server;
            mail_properties.smtp_port = ff::settings.smtp_port;

            limhamn::smtp::client::client{mail_properties}; // send the email

#if FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Activation URL: " + activation_url + "\n");
            logger.write_to_log(limhamn::logger::type::notice, "Sent successfully!\n");
#endif
        } catch (const std::exception& e) {
            return ff::AccountCreationStatus::Failure;
        }
    }

    return ff::AccountCreationStatus::Success;
}


ff::ProfileUpdateStatus ff::update_profile(const limhamn::http::server::request& request, database& db) {
    std::string json{};
    std::string username{};
    std::string icon_path{};
    bool auth{false};

    if (username_is_stored(request)) { // is session cookie
        username = request.session.at("username");
        const std::string key = request.session.at("key");

        if (!verify_key(db, username, key)) {
            return ff::ProfileUpdateStatus::InvalidCreds;
        }
        auth = true;
    }

#ifdef FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to upload a file.\n");
#endif

    const auto file_handles = limhamn::http::utils::parse_multipart_form_file(request.raw_body, settings.temp_directory + "/%f-%h-%r");
    for (const auto& it : file_handles) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "File name: " + it.filename + ", Name: " + it.name + "\n");
#endif
        if (it.name == "json") {
            json = ff::open_file(it.path);
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got JSON\n");
#endif
        } else if (it.name == "icon") {
            icon_path = it.path;
#ifdef FF_DEBUG
            logger.write_to_log(limhamn::logger::type::notice, "Got icon\n");
#endif
        }
    }

    if (json.empty()) {
        return ff::ProfileUpdateStatus::InvalidJson;
    }

    nlohmann::json user_json;
    try {
        user_json = nlohmann::json::parse(json);
    } catch (const std::exception&) {
        return ff::ProfileUpdateStatus::InvalidJson;
    }

    if (!auth) {
        if (user_json.find("username") == user_json.end() || !user_json.at("username").is_string()) {
            return ff::ProfileUpdateStatus::InvalidCreds;
        }
        if (user_json.find("key") == user_json.end() || !user_json.at("key").is_string()) {
            return ff::ProfileUpdateStatus::InvalidCreds;
        }

        username = user_json.at("username").get<std::string>();
        const std::string key = user_json.at("key").get<std::string>();

        if (!verify_key(db, username, key)) {
            return ff::ProfileUpdateStatus::InvalidCreds;
        }
    }

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "users", "username", username));
    } catch (const std::exception&) {
        return ff::ProfileUpdateStatus::Failure;
    }

    if (!icon_path.empty()) {
        if (!ff::validate_image(icon_path)) {
            return ff::ProfileUpdateStatus::InvalidIcon;
        }

        if (!ff::convert_to_webp(icon_path, icon_path)) {
            return ff::ProfileUpdateStatus::Failure;
        }

        std::string icon_key = ff::upload_file(db, FileConstruct{
            .path = icon_path,
            .name = "icon.webp",
            .username = username,
            .ip_address = request.ip_address,
            .user_agent = request.user_agent,
        });

        if (icon_key.empty()) {
            return ff::ProfileUpdateStatus::Failure;
        }

        db_json["profile"]["icon_key"] = icon_key;
    }
    if (user_json.find("description") != user_json.end() && user_json.at("description").is_string()) {
        db_json["profile"]["description"] = limhamn::http::utils::htmlspecialchars(user_json.at("description").get<std::string>());
    }

    return ff::ProfileUpdateStatus::Success;
}