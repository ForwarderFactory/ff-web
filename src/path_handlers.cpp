#include <filesystem>
#include <ff.hpp>
#include <scrypto.hpp>
#include <limhamn/http/http_utils.hpp>
#include <nlohmann/json.hpp>
#include <maddy/parser.h>

limhamn::http::server::response ff::handle_root_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    const auto prepare_file = [](const std::string& path) -> std::string {
        static const std::string temp_file = settings.temp_directory + "/index.html";

        if (static_exists.is_file(temp_file)) {
            return temp_file;
        }

        std::filesystem::remove(temp_file);
        std::filesystem::copy_file(path, temp_file);

        // get domain from site url
        std::string domain = settings.site_url;
        if (domain.find("https://") != std::string::npos) {
            domain = domain.substr(8);
        } else if (domain.find("http://") != std::string::npos) {
            domain = domain.substr(7);
        }
        if (domain.find('/') != std::string::npos) {
            domain = domain.substr(0, domain.find('/'));
        }
        const std::vector<std::pair<std::string, std::string>> find_replace_table = {
            {"{{ff_title}}", settings.title},
            {"{{ff_description}}", settings.description},
            {"{{ff_domain}}", domain},
            {"{{ff_favicon_path}}", virtual_favicon_path},
            {"{{ff_css_path}}", virtual_stylesheet_path},
            {"{{ff_js_path}}", virtual_script_path},
                {"{{ff_body_replace}}", needs_setup ? "<script>setup();</script>" : ""},
            {"\n", ""},
            {"\t", ""},
        };

        auto contents = open_file(temp_file);
        for (const auto& it : find_replace_table) {
            size_t pos = 0;
            while ((pos = contents.find(it.first, pos)) != std::string::npos) {
                contents.replace(pos, it.first.length(), it.second);
                pos += it.second.length();
            }
        }

        std::ofstream file(temp_file, std::ios::out);
        file << contents;
        file.close();

        return temp_file;
    };

    response.content_type = "text/html";
    response.http_status = 200;
    response.body = ff::cache_manager.open_file(prepare_file(settings.html_file));

    return response;
}

limhamn::http::server::response ff::handle_try_upload_forwarder_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_NO_BODY";
        json["error_str"] = "No body.";
        response.body = json.dump();
        return response;
    }

    const std::pair<ff::UploadStatus, std::string> status = ff::try_upload_forwarder(request, db);

    if (status.first == UploadStatus::Success) {
        nlohmann::json json;
        json["id"] = status.second;
        response.content_type = "application/json";
        response.http_status = 200;
        response.body = json.dump();
    } else {
        static const std::unordered_map<UploadStatus, std::pair<std::string, std::string>> status_map{
            {UploadStatus::NoFile, {"FF_NO_FILE", "No file was specified."}},
            {UploadStatus::Failure, {"FF_FAILURE", "Failed to upload the file."}},
            {UploadStatus::TooLarge, {"FF_TOO_LARGE", "The file was too large."}},
            {UploadStatus::InvalidCreds, {"FF_INVALID_CREDS", "Invalid credentials."}},
        };
        nlohmann::json json;
        if (status_map.find(status.first) != status_map.end()) {
            json["error"] = status_map.at(status.first).first;
            json["error_str"] = status_map.at(status.first).second;
        } else {
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
        }

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
    }

    return response;
}

limhamn::http::server::response ff::handle_try_upload_file_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_NO_BODY";
        json["error_str"] = "No body.";
        response.body = json.dump();
        return response;
    }

    const std::pair<ff::UploadStatus, std::string> status = ff::try_upload_file(request, db);

    if (status.first == UploadStatus::Success) {
        nlohmann::json json;
        json["id"] = status.second;
        response.content_type = "application/json";
        response.http_status = 200;
        response.body = json.dump();
    } else {
        static const std::unordered_map<UploadStatus, std::pair<std::string, std::string>> status_map{
            {UploadStatus::NoFile, {"FF_NO_FILE", "No file was specified."}},
            {UploadStatus::Failure, {"FF_FAILURE", "Failed to upload the file."}},
            {UploadStatus::TooLarge, {"FF_TOO_LARGE", "The file was too large."}},
            {UploadStatus::InvalidCreds, {"FF_INVALID_CREDS", "Invalid credentials."}},
        };
        nlohmann::json json;
        if (status_map.find(status.first) != status_map.end()) {
            json["error"] = status_map.at(status.first).first;
            json["error_str"] = status_map.at(status.first).second;
        } else {
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
        }

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
    }

    return response;
}

limhamn::http::server::response ff::handle_setup_endpoint(const limhamn::http::server::request& request, database& db) {
    return handle_root_endpoint(request, db);
}

limhamn::http::server::response ff::handle_try_setup_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    if (!ff::needs_setup) {
        response.location = "/";
        return response;
    }

    if (request.method != "POST") {
        response.http_status = 405;
        nlohmann::json json;
        json["error"] = "FF_METHOD_NOT_ALLOWED";
        json["error_str"] = "Method not allowed.";
        response.body = json.dump();
        return response;
    }
    if (request.body.empty()) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_NO_BODY";
        json["error_str"] = "No body.";
        response.body = json.dump();
        return response;
    }

    nlohmann::json input_json;
    try {
        input_json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON.";
        response.body = json.dump();
        return response;
    }

    if (input_json.find("username") == input_json.end() || !input_json.at("username").is_string()) {
        nlohmann::json json;
        json["error"] = "FF_NO_USERNAME";
        json["error_str"] = "No username.";
        response.body = json.dump();
        return response;
    }
    if (input_json.find("password") == input_json.end() || !input_json.at("password").is_string()) {
        nlohmann::json json;
        json["error"] = "FF_NO_PASSWORD";
        json["error_str"] = "No password.";
        response.body = json.dump();
        return response;
    }
    if (input_json.find("email") == input_json.end() || !input_json.at("email").is_string()) {
        nlohmann::json json;
        json["error"] = "FF_NO_EMAIL";
        json["error_str"] = "No email.";
        response.body = json.dump();
        return response;
    }

    const std::string& username = input_json.at("username").get<std::string>();
    const std::string& password = input_json.at("password").get<std::string>();
    const std::string& ip_address = request.ip_address;
    const std::string& user_agent = request.user_agent;
    const std::string& email = input_json.at("email").get<std::string>();

    AccountCreationStatus status = ff::make_account(
            db,
            username,
            password,
            email,
            ip_address,
            user_agent,
            UserType::Administrator
    );

    if (status == AccountCreationStatus::Success) {
        std::filesystem::remove(settings.temp_directory + "/index.html");
        ff::cache_manager = ff::CacheManager{};
        ff::static_exists = ff::StaticExists{};
        ff::needs_setup = false;
        response.http_status = 204;
        return response;
    } else {
        const std::unordered_map<AccountCreationStatus, std::pair<std::string, std::string>> error_map{
            {AccountCreationStatus::Failure, {"FF_FAILURE", "Failure."}},
            {AccountCreationStatus::UsernameExists, {"FF_USERNAME_EXISTS", "Username exists."}},
            {AccountCreationStatus::UsernameTooShort, {"FF_USERNAME_TOO_SHORT", "Username too short."}},
            {AccountCreationStatus::UsernameTooLong, {"FF_USERNAME_TOO_LONG", "Username too long."}},
            {AccountCreationStatus::PasswordTooShort, {"FF_PASSWORD_TOO_SHORT", "Password too short."}},
            {AccountCreationStatus::PasswordTooLong, {"FF_PASSWORD_TOO_LONG", "Password too long."}},
            {AccountCreationStatus::InvalidUsername, {"FF_INVALID_USERNAME", "Invalid username."}},
            {AccountCreationStatus::InvalidPassword, {"FF_INVALID_PASSWORD", "Invalid password."}},
        };

        if (error_map.find(status) != error_map.end()) {
            nlohmann::json json;
            json["error"] = error_map.at(status).first;
            json["error_str"] = error_map.at(status).second;
            response.body = json.dump();
        } else {
            nlohmann::json json;
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
            response.body = json.dump();
        }

        response.http_status = 400;
        return response;
    }
}

limhamn::http::server::response ff::handle_virtual_favicon_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    response.content_type = "image/svg+xml";
    response.http_status = 200;

    if (settings.favicon_file.empty() || !static_exists.is_file(settings.favicon_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    response.body = ff::cache_manager.open_file(settings.favicon_file);

    return response;
}

limhamn::http::server::response ff::handle_virtual_stylesheet_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    response.content_type = "text/css";
    response.http_status = 200;

    if (settings.css_file.empty() || !static_exists.is_file(settings.css_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    response.body = ff::cache_manager.open_file(settings.css_file);

    return response;
}

limhamn::http::server::response ff::handle_virtual_script_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response;

    response.content_type = "text/javascript";
    response.http_status = 200;

    // TODO: Just like the name, this function is UGLY AS FUCK, and does not belong anywhere near
    // a project like this. But I simply cannot be bothered to write a JS minifier myself, nor
    // am I aware of any C++ library for doing such a thing, and I am therefore just going to call uglifyjs.
#ifndef FF_DEBUG
    const auto uglify_file = [](const std::string& path) -> std::string {
        static const std::string temp_file = settings.temp_directory + "/ff_temp.js";
        if (static_exists.is_file(temp_file)) {
            return temp_file;
        }
        if (std::system("which uglifyjs > /dev/null") != 0) {
            return path;
        }

        std::filesystem::remove(temp_file);
        std::filesystem::copy_file(path, temp_file);

        // run uglifyjs on the file
        std::string command = "uglifyjs " + temp_file + " -o " + temp_file;
        std::system(command.c_str());

        return temp_file;
    };
#endif

#if FF_DEBUG
    response.body = ff::cache_manager.open_file(settings.script_file);
#else
    std::string path = uglify_file(settings.script_file);
    response.body = ff::cache_manager.open_file(path);
#endif

    return response;
}

limhamn::http::server::response ff::handle_api_try_register_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

#if FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to register.\n");
    logger.write_to_log(limhamn::logger::type::notice, "Request body: " + request.body + "\n");
#endif

    if (request.method != "POST") {
        nlohmann::json json;
        json["error"] = "FF_METHOD_NOT_ALLOWED";
        json["error_str"] = "Method not allowed.";
        response.body = json.dump();
        response.http_status = 405;
        return response;
    }

    if (ff::settings.public_registration == false) {
        response.http_status = 403;
        nlohmann::json json;
        json["error"] = "FF_FORBIDDEN";
        json["error_str"] = "Registration is disabled.";
        response.body = json.dump();
        return response;
    }

    nlohmann::json input_json;
    try {
        input_json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON.";
        response.body = json.dump();
        return response;
    }

    if (input_json.find("username") == input_json.end() || !input_json.at("username").is_string()) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_MISSING_USERNAME";
        json["error_str"] = "Username missing.";
        response.body = json.dump();
        return response;
    } else if (input_json.find("password") == input_json.end() || !input_json.at("password").is_string()) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_MISSING_PASSWORD";
        json["error_str"] = "Password missing.";
        response.body = json.dump();
        return response;
    }

    const std::string& username = input_json.at("username");
    const std::string& password = input_json.at("password");
    std::string email{};

    if (settings.enable_email_verification && (input_json.find("email") == input_json.end() || !input_json.at("email").is_string())) {
        response.http_status = 400;
        nlohmann::json json;
        json["error"] = "FF_MISSING_EMAIL";
        json["error_str"] = "Email missing.";
        response.body = json.dump();
        return response;
    } else if (settings.enable_email_verification) {
        email = input_json.at("email");
    }

    const std::string& ip_address = request.ip_address;
    const std::string& user_agent = request.user_agent;

    AccountCreationStatus status = ff::make_account(
            db,
            username,
            password,
            email,
            ip_address,
            user_agent,
            ff::UserType::User
    );

    if (status == AccountCreationStatus::Success) {
        if (settings.enable_email_verification) {
            response.http_status = 200;
            nlohmann::json json;
            json["note"] = "FF_EMAIL_VERIFICATION_REQUIRED";
            json["note_str"] = "Email verification required.";
            response.body = json.dump();
            return response;
        }
        response.http_status = 204;
    } else {
        static const std::unordered_map<AccountCreationStatus, std::pair<std::string, std::string>> map{
            {AccountCreationStatus::Failure, {"FF_FAILURE", "Failure."}},
            {AccountCreationStatus::UsernameExists, {"FF_USERNAME_EXISTS", "Username exists."}},
            {AccountCreationStatus::UsernameTooShort, {"FF_USERNAME_TOO_SHORT", "Username too short."}},
            {AccountCreationStatus::UsernameTooLong, {"FF_USERNAME_TOO_LONG", "Username too long."}},
            {AccountCreationStatus::PasswordTooShort, {"FF_PASSWORD_TOO_SHORT", "Password too short."}},
            {AccountCreationStatus::PasswordTooLong, {"FF_PASSWORD_TOO_LONG", "Password too long."}},
            {AccountCreationStatus::InvalidUsername, {"FF_INVALID_USERNAME", "Invalid username."}},
            {AccountCreationStatus::InvalidPassword, {"FF_INVALID_PASSWORD", "Invalid password."}},
            {AccountCreationStatus::InvalidEmail, {"FF_INVALID_EMAIL", "Invalid email."}},
            {AccountCreationStatus::EmailExists, {"FF_EMAIL_EXISTS", "Email exists."}},
        };

        if (map.find(status) == map.end()) {
            nlohmann::json json;
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
            response.body = json.dump();
            response.http_status = 500;
            return response;
        }

        nlohmann::json json;
        json["error"] = map.at(status).first;
        json["error_str"] = map.at(status).second;
        response.body = json.dump();
        response.http_status = 400;

        return response;
    }

    return response;
}

limhamn::http::server::response ff::handle_api_try_login_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    if (request.method != "POST") {
        response.http_status = 405;
        nlohmann::json json;
        json["error"] = "FF_METHOD_NOT_ALLOWED";
        json["error_str"] = "Method not allowed.";
        response.body = json.dump();
        return response;
    }

    nlohmann::json input_json;
    try {
       input_json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        response.content_type = "application/json";
        response.http_status = 400;

        return response;
    }

#if FF_DEBUG
    logger.write_to_log(limhamn::logger::type::notice, "Attempting to login.\n");
    logger.write_to_log(limhamn::logger::type::notice, "Request body: " + request.body + "\n");
#endif

    std::string username{};

    if (input_json.find("password") == input_json.end() || !input_json.at("password").is_string()) {
        nlohmann::json json;
        json["error"] = "FF_MISSING_PASSWORD";
        json["error_str"] = "Password missing.";
        response.body = json.dump();
        response.http_status = 400;
        return response;
    }

    if (input_json.find("username") != input_json.end() && input_json.at("username").is_string()) {
        username = input_json.at("username").get<std::string>();
    } else if (input_json.find("email") != input_json.end() && input_json.at("email").is_string()) {
        try {
            username = get_username_from_email(db, input_json.at("email").get<std::string>());
        } catch (const std::exception&) {
            nlohmann::json json;
            json["error"] = "FF_INVALID_EMAIL";
            json["error_str"] = "Invalid email.";
            response.body = json.dump();
            response.http_status = 400;
            return response;
        }
    } else {
        nlohmann::json json;
        json["error"] = "FF_MISSING_USERNAME";
        json["error_str"] = "Username missing.";
        response.body = json.dump();
        response.http_status = 400;
        return response;
    }

    const std::string& password = input_json.at("password").get<std::string>();
    const std::string& ip_address = request.ip_address;
    const std::string& user_agent = request.user_agent;

    std::pair<LoginStatus, std::string> status = ff::try_login(
            db,
            username,
            password,
            ip_address,
            user_agent,
            response
    );

    if (status.first == LoginStatus::Success) {
        response.http_status = 200;
        nlohmann::json json;
        json["username"] = username;
        json["key"] = status.second;
        response.body = json.dump();
        return response;
    } else {
        const std::unordered_map<LoginStatus, std::pair<std::string, std::string>> error_map{
            {LoginStatus::Failure, {"FF_FAILURE", "Failure."}},
            {LoginStatus::InvalidUsername, {"FF_INVALID_USERNAME", "Invalid username."}},
            {LoginStatus::InvalidPassword, {"FF_INVALID_PASSWORD", "Invalid password."}},
            {LoginStatus::Inactive, {"FF_NOT_ACTIVATED", "Account not activated. Check your email!"}},
            {LoginStatus::Banned, {"FF_BANNED", "Banned."}},
        };

        if (error_map.find(status.first) == error_map.end()) {
            nlohmann::json json;
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
            response.body = json.dump();
            response.http_status = 500;
            return response;
        }

        nlohmann::json json;
        json["error"] = error_map.at(status.first).first;
        json["error_str"] = error_map.at(status.first).second;
        response.body = json.dump();
        response.http_status = 400;
    }

    return response;
}

limhamn::http::server::response ff::handle_api_get_forwarders_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    response.content_type = "application/json";
    response.http_status = 200;

    // find filter in the json
    struct Filter {
        bool is_forwarder{false}; // if true, must be a forwarder
        bool accepted{false}; // if true, must be accepted
        bool needs_review{false}; // if true, must need review
        std::string search_string{}; // if not empty, must contain this string somewhere
        std::string title_id_string{}; // if not empty, must match this title id
        std::string uploader{}; // if not empty, must match this uploader
        std::string author{}; // if not empty, must match this author
        int type{-1}; // if not -1, must match this type (1 = channel, 0 = forwarder)
        std::vector<std::string> categories{}; // if not empty, must match one of these categories
        std::string location{}; // if not empty, must match this location
        int64_t submitted_before{-1}; // if not -1, must be submitted before this time
        int64_t submitted_after{-1}; // if not -1, must be submitted after this time
        std::pair<int64_t, int64_t> submitted_between{-1, -1}; // if not -1, must be submitted between these times
        int vwii{-1}; // if not -1, must match this vwii (-1 = any, 0 = no vwii, 1 = vwii)
        int begin{-1}; // if not -1, start at this index
        int end{-1}; // if not -1, end at this index
        std::string identifier{}; // if not empty, must match this identifier
    };

    Filter filter{};

    if (request.method == "POST" && !request.body.empty()) {
        nlohmann::json input_json;

        try {
            input_json = nlohmann::json::parse(request.body);
        } catch (const std::exception&) {
            response.http_status = 400;
            response.body = "Bad Request";

            return response;
        }

        // parse and override the filter
        if (input_json.find("filter") != input_json.end() && input_json.at("filter").is_object()) {
            if (input_json.at("filter").find("is_forwarder") != input_json.at("filter").end() && input_json.at("filter").at("is_forwarder").is_boolean()) {
                filter.is_forwarder = input_json.at("filter").at("is_forwarder").get<bool>();
            }
            if (input_json.at("filter").find("accepted") != input_json.at("filter").end() && input_json.at("filter").at("accepted").is_boolean()) {
                filter.accepted = input_json.at("filter").at("accepted").get<bool>();
            }
            if (input_json.at("filter").find("needs_review") != input_json.at("filter").end() && input_json.at("filter").at("needs_review").is_boolean()) {
                filter.needs_review = input_json.at("filter").at("needs_review").get<bool>();
            }
            if (input_json.at("filter").find("search_string") != input_json.at("filter").end() && input_json.at("filter").at("search_string").is_string()) {
                filter.search_string = input_json.at("filter").at("search_string").get<std::string>();
            }
            if (input_json.at("filter").find("title_id_string") != input_json.at("filter").end() && input_json.at("filter").at("title_id_string").is_string()) {
                filter.title_id_string = input_json.at("filter").at("title_id_string").get<std::string>();
            }
            if (input_json.at("filter").find("uploader") != input_json.at("filter").end() && input_json.at("filter").at("uploader").is_string()) {
                filter.uploader = input_json.at("filter").at("uploader").get<std::string>();
            }
            if (input_json.at("filter").find("author") != input_json.at("filter").end() && input_json.at("filter").at("author").is_string()) {
                filter.author = input_json.at("filter").at("author").get<std::string>();
            }
            if (input_json.at("filter").find("type") != input_json.at("filter").end() && input_json.at("filter").at("type").is_number_integer()) {
                filter.type = input_json.at("filter").at("type").get<int>();
                if (filter.type != -1 && filter.type != 0 && filter.type != 1) {
                    filter.type = -1;
                }
            }
            if (input_json.at("filter").find("categories") != input_json.at("filter").end() && input_json.at("filter").at("categories").is_array()) {
                for (const auto& it : input_json.at("filter").at("categories")) {
                    if (it.is_string()) {
                        filter.categories.push_back(it.get<std::string>());
                    }
                }
            }
            if (input_json.at("filter").find("location") != input_json.at("filter").end() && input_json.at("filter").at("location").is_string()) {
                filter.location = input_json.at("filter").at("location").get<std::string>();
            }
            if (input_json.at("filter").find("submitted_before") != input_json.at("filter").end() && input_json.at("filter").at("submitted_before").is_number_integer()) {
                filter.submitted_before = input_json.at("filter").at("submitted_before").get<int64_t>();
            }
            if (input_json.at("filter").find("submitted_after") != input_json.at("filter").end() && input_json.at("filter").at("submitted_after").is_number_integer()) {
                filter.submitted_after = input_json.at("filter").at("submitted_after").get<int64_t>();
            }
            if (input_json.at("filter").find("submitted_between") != input_json.at("filter").end() && input_json.at("filter").at("submitted_between").is_array()) {
                if (input_json.at("filter").at("submitted_between").size() == 2) {
                    if (input_json.at("filter").at("submitted_between").at(0).is_number_integer() && input_json.at("filter").at("submitted_between").at(1).is_number_integer()) {
                        filter.submitted_between = std::make_pair(input_json.at("filter").at("submitted_between").at(0).get<int64_t>(), input_json.at("filter").at("submitted_between").at(1).get<int64_t>());
                    }
                }
            }
            if (input_json.at("filter").find("vwii") != input_json.at("filter").end() && input_json.at("filter").at("vwii").is_number_integer()) {
                filter.vwii = input_json.at("filter").at("vwii").get<int>();
                if (filter.vwii != 0 && filter.vwii != 1) {
                    filter.vwii = -1;
                }
            }
            if (input_json.at("filter").find("begin") != input_json.at("filter").end() && input_json.at("filter").at("begin").is_number_integer()) {
                filter.begin = input_json.at("filter").at("begin").get<int>();
            }
            if (input_json.at("filter").find("end") != input_json.at("filter").end() && input_json.at("filter").at("end").is_number_integer()) {
                filter.end = input_json.at("filter").at("end").get<int>();
            }
            if (input_json.at("filter").find("identifier") != input_json.at("filter").end() && input_json.at("filter").at("identifier").is_string()) {
                filter.identifier = input_json.at("filter").at("identifier").get<std::string>();
            }
        }
    }

    nlohmann::json json{};

    json["forwarders"] = nlohmann::json::array();

    const auto get_forwarders = [&]() -> void {
        nlohmann::json forwarders_json;

        std::vector<std::unordered_map<std::string, std::string>> forwarders;
        if (!filter.identifier.empty()) {
            forwarders = db.query("SELECT * FROM forwarders WHERE identifier = ?;", filter.identifier);
        } else {
            forwarders = db.query("SELECT * FROM forwarders;");
        }

        int i = 0;
        for (const auto& it : forwarders) {
            if (filter.begin != -1 && i < filter.begin) {
                continue;
            } else if (filter.end != -1 && i > filter.end) {
                break;
            }

            try {
                forwarders_json = nlohmann::json::parse(it.at("json"));
            } catch (const std::exception&) {
                continue;
            }

            if (forwarders_json.find("meta") == forwarders_json.end()) {
                continue;
            }

            nlohmann::json meta;
            try {
                meta = forwarders_json.at("meta");
            } catch (const std::exception&) {
                return;
            }

            if (filter.accepted && forwarders_json.find("needs_review") != forwarders_json.end() && forwarders_json.at("needs_review").is_boolean()) {
                if (forwarders_json.at("needs_review").get<bool>()) {
                    continue;
                }
            }
            if (filter.needs_review && forwarders_json.find("needs_review") != forwarders_json.end() && forwarders_json.at("needs_review").is_boolean()) {
                if (!forwarders_json.at("needs_review").get<bool>()) {
                    continue;
                }
            }
            if (!filter.search_string.empty()) {
                std::string full_str{};

                if (meta.find("title") != meta.end() && meta.at("title").is_string()) {
                    full_str += meta.at("title").get<std::string>();
                }
                if (meta.find("author") != meta.end() && meta.at("author").is_string()) {
                    full_str += meta.at("author").get<std::string>();
                }
                if (meta.find("description") != meta.end() && meta.at("description").is_string()) {
                    full_str += meta.at("description").get<std::string>();
                }
                if (meta.find("title_id") != meta.end() && meta.at("title_id").is_string()) {
                    full_str += meta.at("title_id").get<std::string>();
                }
                if (forwarders_json.find("uploader") != forwarders_json.end() && forwarders_json.at("uploader").is_string()) {
                    full_str += forwarders_json.at("uploader").get<std::string>();
                }

                std::transform(full_str.begin(), full_str.end(), full_str.begin(), ::tolower);
                std::transform(filter.search_string.begin(), filter.search_string.end(), filter.search_string.begin(), ::tolower);

                if (full_str.find(filter.search_string) == std::string::npos) {
                    continue;
                }
            }
            if (!filter.uploader.empty()) {
                if (forwarders_json.find("uploader") != forwarders_json.end() && forwarders_json.at("uploader").is_string()) {
                    std::transform(filter.uploader.begin(), filter.uploader.end(), filter.uploader.begin(), ::tolower);
                    std::string uploader{forwarders_json.at("uploader").get<std::string>()};
                    std::transform(uploader.begin(), uploader.end(), uploader.begin(), ::tolower);
                    if (uploader != filter.uploader) {
                        continue;
                    }
                }
            }
            if (!filter.author.empty()) {
                if (meta.find("author") != meta.end() && meta.at("author").is_string()) {
                    std::transform(filter.author.begin(), filter.author.end(), filter.author.begin(), ::tolower);
                    std::string author{meta.at("author").get<std::string>()};
                    std::transform(author.begin(), author.end(), author.begin(), ::tolower);
                    if (author != filter.author) {
                        continue;
                    }
                }
            }
            if (filter.type != -1) {
                if (meta.find("type") != meta.end() && meta.at("type").is_number_integer()) {
                    if (meta.at("type").get<int>() != filter.type) {
                        continue;
                    }
                }
            }

            if (!filter.categories.empty()) {
                bool found = false;

                for (const auto& category : filter.categories) {
                    if (meta.find("categories") != meta.end() && meta.at("categories").is_array()) {
                        for (const auto& it_category : meta.at("categories")) {
                            if (it_category.is_string()) {
                                std::string cat{it_category.get<std::string>()};
                                std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
                                if (cat == category) {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (found) break;
                    }
                }

                if (!found) {
                    continue;
                }
            }
            if (!filter.location.empty()) {
                if (meta.find("location") != meta.end() && meta.at("location").is_string()) {
                    std::transform(filter.location.begin(), filter.location.end(), filter.location.begin(), ::tolower);
                    std::string location{meta.at("location").get<std::string>()};
                    std::transform(location.begin(), location.end(), location.begin(), ::tolower);
                    if (location != filter.location) {
                        continue;
                    }
                }
            }
            if (filter.submitted_before != -1) {
                if (forwarders_json.find("submitted") != forwarders_json.end() && forwarders_json.at("submitted").is_number_integer()) {
                    if (forwarders_json.at("submitted").get<int64_t>() > filter.submitted_before) {
                        continue;
                    }
                }
            }
            if (filter.submitted_after != -1) {
                if (forwarders_json.find("submitted") != forwarders_json.end() && forwarders_json.at("submitted").is_number_integer()) {
                    if (forwarders_json.at("submitted").get<int64_t>() < filter.submitted_after) {
                        continue;
                    }
                }
            }
            if (filter.submitted_between.first != -1 && filter.submitted_between.second != -1) {
                if (forwarders_json.find("submitted") != forwarders_json.end() && forwarders_json.at("submitted").is_number_integer()) {
                    if (forwarders_json.at("submitted").get<int64_t>() < filter.submitted_between.first || forwarders_json.at("submitted").get<int64_t>() > filter.submitted_between.second) {
                        continue;
                    }
                }
            }
            if (filter.vwii != -1) {
                if (meta.find("vwii_compatible") != meta.end() && meta.at("vwii_compatible").is_boolean()) {
                    if (meta.at("vwii_compatible").get<bool>() != filter.vwii) {
                        continue;
                    }
                }
            }

            // replace [ratings] with a single average integer from [ratings][username][rating]
            if (forwarders_json.find("ratings") != forwarders_json.end() && forwarders_json.at("ratings").is_object()) {
                int total_rating = 0;
                int count = 0;

                for (const auto& _it : forwarders_json.at("ratings").items()) {
                    if (_it.value().at("rating").is_number_integer()) {
                        total_rating += _it.value().at("rating").get<int>();
                        ++count;
                    }
                }

                if (count > 0) {
                    forwarders_json["average_rating"] = total_rating / count;
                    forwarders_json["rating_count"] = count;
                    forwarders_json["ratings"] = nlohmann::json::object();
                } else {
                    forwarders_json["average_rating"] = 0;
                    forwarders_json["rating_count"] = 0;
                    forwarders_json["ratings"] = nlohmann::json::object();
                }
            } else {
                forwarders_json["average_rating"] = 0;
                forwarders_json["rating_count"] = 0;
                forwarders_json["ratings"] = nlohmann::json::object();
            }

            json["forwarders"].push_back(forwarders_json);
            ++i;
        }
    };

    get_forwarders();

    response.body = json.dump();

    return response;
}

limhamn::http::server::response ff::handle_api_get_files_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    response.content_type = "application/json";
    response.http_status = 200;

    // find filter in the json
    struct Filter {
        bool accepted{false}; // if true, must be accepted
        bool needs_review{false}; // if true, must need review
        std::string search_string{}; // if not empty, must contain this string somewhere
        std::string filename{}; // if not empty, must have this file
        std::string title{}; // if not empty, must contain this title
        std::string uploader{}; // if not empty, must match this uploader
        std::string author{}; // if not empty, must match this author
        std::vector<std::string> categories{}; // if not empty, must match one of these categories
        int64_t submitted_before{-1}; // if not -1, must be submitted before this time
        int64_t submitted_after{-1}; // if not -1, must be submitted after this time
        std::pair<int64_t, int64_t> submitted_between{-1, -1}; // if not -1, must be submitted between these times
        int begin{-1}; // if not -1, start at this index
        int end{-1}; // if not -1, end at this index
        std::string identifier{}; // if not empty, must match this identifier
    };

    Filter filter{};

    if (request.method == "POST" && !request.body.empty()) {
        nlohmann::json input_json;

        try {
            input_json = nlohmann::json::parse(request.body);
        } catch (const std::exception&) {
            response.http_status = 400;
            response.body = "Bad Request";

            return response;
        }

        // parse and override the filter
        if (input_json.find("filter") != input_json.end() && input_json.at("filter").is_object()) {
            if (input_json.at("filter").find("accepted") != input_json.at("filter").end() && input_json.at("filter").at("accepted").is_boolean()) {
                filter.accepted = input_json.at("filter").at("accepted").get<bool>();
            }
            if (input_json.at("filter").find("needs_review") != input_json.at("filter").end() && input_json.at("filter").at("needs_review").is_boolean()) {
                filter.needs_review = input_json.at("filter").at("needs_review").get<bool>();
            }
            if (input_json.at("filter").find("search_string") != input_json.at("filter").end() && input_json.at("filter").at("search_string").is_string()) {
                filter.search_string = input_json.at("filter").at("search_string").get<std::string>();
            }
            if (input_json.at("filter").find("filename") != input_json.at("filter").end() && input_json.at("filter").at("filename").is_string()) {
                filter.filename = input_json.at("filter").at("filename").get<std::string>();
            }
            if (input_json.at("filter").find("title") != input_json.at("filter").end() && input_json.at("filter").at("title").is_string()) {
                filter.title = input_json.at("filter").at("title").get<std::string>();
            }
            if (input_json.at("filter").find("uploader") != input_json.at("filter").end() && input_json.at("filter").at("uploader").is_string()) {
                filter.uploader = input_json.at("filter").at("uploader").get<std::string>();
            }
            if (input_json.at("filter").find("author") != input_json.at("filter").end() && input_json.at("filter").at("author").is_string()) {
                filter.author = input_json.at("filter").at("author").get<std::string>();
            }
            if (input_json.at("filter").find("categories") != input_json.at("filter").end() && input_json.at("filter").at("categories").is_array()) {
                for (const auto& it : input_json.at("filter").at("categories")) {
                    if (it.is_string()) {
                        filter.categories.push_back(it.get<std::string>());
                    }
                }
            }
            if (input_json.at("filter").find("submitted_before") != input_json.at("filter").end() && input_json.at("filter").at("submitted_before").is_number_integer()) {
                filter.submitted_before = input_json.at("filter").at("submitted_before").get<int64_t>();
            }
            if (input_json.at("filter").find("submitted_after") != input_json.at("filter").end() && input_json.at("filter").at("submitted_after").is_number_integer()) {
                filter.submitted_after = input_json.at("filter").at("submitted_after").get<int64_t>();
            }
            if (input_json.at("filter").find("submitted_between") != input_json.at("filter").end() && input_json.at("filter").at("submitted_between").is_array()) {
                if (input_json.at("filter").at("submitted_between").size() == 2) {
                    if (input_json.at("filter").at("submitted_between").at(0).is_number_integer() && input_json.at("filter").at("submitted_between").at(1).is_number_integer()) {
                        filter.submitted_between = std::make_pair(input_json.at("filter").at("submitted_between").at(0).get<int64_t>(), input_json.at("filter").at("submitted_between").at(1).get<int64_t>());
                    }
                }
            }
            if (input_json.at("filter").find("begin") != input_json.at("filter").end() && input_json.at("filter").at("begin").is_number_integer()) {
                filter.begin = input_json.at("filter").at("begin").get<int>();
            }
            if (input_json.at("filter").find("end") != input_json.at("filter").end() && input_json.at("filter").at("end").is_number_integer()) {
                filter.end = input_json.at("filter").at("end").get<int>();
            }
            if (input_json.at("filter").find("identifier") != input_json.at("filter").end() && input_json.at("filter").at("identifier").is_string()) {
                filter.identifier = input_json.at("filter").at("identifier").get<std::string>();
            }
        }
    }

    nlohmann::json json{};

    json["files"] = nlohmann::json::array();

    const auto get_files = [&]() -> void {
        nlohmann::json files_json;

        std::vector<std::unordered_map<std::string, std::string>> files;
        if (!filter.identifier.empty()) {
            files = db.query("SELECT * FROM sandbox WHERE identifier = ?;", filter.identifier);
        } else {
            files = db.query("SELECT * FROM sandbox;");
        }

        int i = 0;
        for (const auto& it : files) {
            if (filter.begin != -1 && i < filter.begin) {
                continue;
            } else if (filter.end != -1 && i > filter.end) {
                break;
            }

            try {
                files_json = nlohmann::json::parse(it.at("json"));
            } catch (const std::exception&) {
                continue;
            }

            if (files_json.find("meta") == files_json.end()) {
                continue;
            }

            nlohmann::json meta;
            try {
                meta = files_json.at("meta");
            } catch (const std::exception&) {
                return;
            }

            if (filter.accepted && files_json.find("needs_review") != files_json.end() && files_json.at("needs_review").is_boolean()) {
                if (files_json.at("needs_review").get<bool>()) {
                    continue;
                }
            }
            if (filter.needs_review && files_json.find("needs_review") != files_json.end() && files_json.at("needs_review").is_boolean()) {
                if (!files_json.at("needs_review").get<bool>()) {
                    continue;
                }
            }
            if (!filter.search_string.empty()) {
                std::string full_str{};

                if (meta.find("title") != meta.end() && meta.at("title").is_string()) {
                    full_str += meta.at("title").get<std::string>();
                }
                if (meta.find("filename") != meta.end() && meta.at("filename").is_string()) {
                    full_str += meta.at("filename").get<std::string>();
                }
                if (meta.find("author") != meta.end() && meta.at("author").is_string()) {
                    full_str += meta.at("author").get<std::string>();
                }
                if (meta.find("description") != meta.end() && meta.at("description").is_string()) {
                    full_str += meta.at("description").get<std::string>();
                }
                if (files_json.find("uploader") != files_json.end() && files_json.at("uploader").is_string()) {
                    full_str += files_json.at("uploader").get<std::string>();
                }

                std::transform(full_str.begin(), full_str.end(), full_str.begin(), ::tolower);
                std::transform(filter.search_string.begin(), filter.search_string.end(), filter.search_string.begin(), ::tolower);

                if (full_str.find(filter.search_string) == std::string::npos) {
                    continue;
                }
            }
            if (!filter.title.empty()) {
                if (meta.find("title") != meta.end() && meta.at("title").is_string()) {
                    std::transform(filter.title.begin(), filter.title.end(), filter.title.begin(), ::tolower);
                    std::string title{meta.at("title").get<std::string>()};
                    std::transform(title.begin(), title.end(), title.begin(), ::tolower);
                    if (title != filter.title) {
                        continue;
                    }
                }
            }
            if (!filter.filename.empty()) {
                if (meta.find("filename") != meta.end() && meta.at("filename").is_string()) {
                    std::transform(filter.filename.begin(), filter.filename.end(), filter.filename.begin(), ::tolower);
                    std::string filename{meta.at("filename").get<std::string>()};
                    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
                    if (filename != filter.filename) {
                        continue;
                    }
                }
            }
            if (!filter.uploader.empty()) {
                if (files_json.find("uploader") != files_json.end() && files_json.at("uploader").is_string()) {
                    std::transform(filter.uploader.begin(), filter.uploader.end(), filter.uploader.begin(), ::tolower);
                    std::string uploader{files_json.at("uploader").get<std::string>()};
                    std::transform(uploader.begin(), uploader.end(), uploader.begin(), ::tolower);
                    if (uploader != filter.uploader) {
                        continue;
                    }
                }
            }
            if (!filter.author.empty()) {
                if (meta.find("author") != meta.end() && meta.at("author").is_string()) {
                    std::transform(filter.author.begin(), filter.author.end(), filter.author.begin(), ::tolower);
                    std::string author{meta.at("author").get<std::string>()};
                    std::transform(author.begin(), author.end(), author.begin(), ::tolower);
                    if (author != filter.author) {
                        continue;
                    }
                }
            }
            if (!filter.categories.empty()) {
                bool found = false;
                for (const auto& category : filter.categories) {
                    if (meta.find("categories") != meta.end() && meta.at("categories").is_array()) {
                        for (const auto& it_category : meta.at("categories")) {
                            if (it_category.is_string()) {
                                std::string cat{it_category.get<std::string>()};
                                std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
                                if (cat == category) {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (found) break;
                    }
                }

                if (!found) {
                    continue;
                }
            }
            if (filter.submitted_before != -1) {
                if (files_json.find("submitted") != files_json.end() && files_json.at("submitted").is_number_integer()) {
                    if (files_json.at("submitted").get<int64_t>() > filter.submitted_before) {
                        continue;
                    }
                }
            }
            if (filter.submitted_after != -1) {
                if (files_json.find("submitted") != files_json.end() && files_json.at("submitted").is_number_integer()) {
                    if (files_json.at("submitted").get<int64_t>() < filter.submitted_after) {
                        continue;
                    }
                }
            }
            if (filter.submitted_between.first != -1 && filter.submitted_between.second != -1) {
                if (files_json.find("submitted") != files_json.end() && files_json.at("submitted").is_number_integer()) {
                    if (files_json.at("submitted").get<int64_t>() < filter.submitted_between.first || files_json.at("submitted").get<int64_t>() > filter.submitted_between.second) {
                        continue;
                    }
                }
            }

            // replace [ratings] with a single average integer from [ratings][username][rating]
            if (files_json.find("ratings") != files_json.end() && files_json.at("ratings").is_object()) {
                int total_rating = 0;
                int count = 0;

                for (const auto& _it : files_json.at("ratings").items()) {
                    if (_it.value().at("rating").is_number_integer()) {
                        total_rating += _it.value().at("rating").get<int>();
                        ++count;
                    }
                }

                if (count > 0) {
                    files_json["average_rating"] = total_rating / count;
                    files_json["rating_count"] = count;
                    files_json["ratings"] = nlohmann::json::object();
                } else {
                    files_json["average_rating"] = 0;
                    files_json["rating_count"] = 0;
                    files_json["ratings"] = nlohmann::json::object();
                }
            } else {
                files_json["average_rating"] = 0;
                files_json["rating_count"] = 0;
                files_json["ratings"] = nlohmann::json::object();
            }

            json["files"].push_back(files_json);
            ++i;
        }
    };

    get_files();

    response.body = json.dump();

    return response;
}

// this endpoint requires auth and cookies
// in the future, we should allow other kinds of auth for this endpoint, so that third party clients can use it
limhamn::http::server::response ff::handle_api_set_approval_for_uploads_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            nlohmann::json json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // do nothing
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            nlohmann::json json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // do nothing
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (ff::get_user_type(db, username) != ff::UserType::Administrator) {
        nlohmann::json json;
        json["error"] = "FF_NOT_AUTHORIZED";
        json["error_str"] = "You are not authorized to access this endpoint.";
        response.http_status = 403;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "No body.";
        json["error"] = "FF_NO_BODY";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }
    if (request.method != "POST") {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Not a POST request.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Not a POST request.";
        json["error"] = "FF_INVALID_METHOD";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Failed to parse JSON.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid JSON received";
        json["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if ((input.find("forwarders") == input.end() || !input.at("forwarders").is_object()) && (input.find("files") == input.end() || !input.at("files").is_object())) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No forwarders object.\n");
#endif
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON received";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json forwarders;
    nlohmann::json files;
    try {
        forwarders = input.at("forwarders");
    } catch (const std::exception&) {}
    try {
        files = input.at("files");
    } catch (const std::exception&) {}

    if (forwarders.empty() && files.empty()) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON received";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    for (const auto& it : forwarders.items()) {
        const std::string& identifier = it.key();
#if FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Identifier: " + identifier + "\n");
#endif
        if (forwarders.find(identifier) == forwarders.end() || !forwarders.at(identifier).is_boolean()) {
            continue;
        }

        bool accepted = forwarders.at(identifier).get<bool>();

        // get json from db
        try {
            if (accepted) {
                nlohmann::json json;
                try {
                    json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", identifier));
                } catch (const std::exception&) {
                    nlohmann::json _json;
                    _json["error_str"] = "Invalid JSON received";
                    _json["error"] = "FF_INVALID_JSON";
                    response.http_status = 400;
                    response.body = _json.dump();
                    return response;
                }

                json["needs_review"] = false;
                ff::set_json_in_table(db, "forwarders", "identifier", identifier, json.dump());
            } else {
                db.exec("DELETE FROM forwarders WHERE identifier = ?;", identifier);
            }
        } catch (const std::exception&) {
#if FF_DEBUG
            logger.write_to_log(limhamn::logger::type::error, "Help I got caught.\n");
#endif
            continue;
        }
    }

    for (const auto& it : files.items()) {
        const std::string& identifier = it.key();
#if FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Identifier: " + identifier + "\n");
#endif
        if (files.find(identifier) == files.end() || !files.at(identifier).is_boolean()) {
            continue;
        }

        bool accepted = files.at(identifier).get<bool>();

        try {
            if (accepted) {
                nlohmann::json json;
                try {
                    json = nlohmann::json::parse(ff::get_json_from_table(db, "sandbox", "identifier", identifier));
                } catch (const std::exception&) {
                    nlohmann::json _json;
                    _json["error_str"] = "Invalid JSON received";
                    _json["error"] = "FF_INVALID_JSON";
                    response.http_status = 400;
                    response.body = _json.dump();
                    return response;
                }

                json["needs_review"] = false;
                ff::set_json_in_table(db, "sandbox", "identifier", identifier, json.dump());
            } else {
                db.exec("DELETE FROM sandbox WHERE identifier = ?;", identifier);
            }
        } catch (const std::exception&) {
#if FF_DEBUG
            logger.write_to_log(limhamn::logger::type::error, "Help I got caught.\n");
#endif
            continue;
        }
    }

    response.http_status = 201;
    return response;
}

limhamn::http::server::response ff::handle_api_update_profile(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error"] = "FF_NO_BODY";
        json["error_str"] = "No body.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    const auto status = ff::update_profile(request, db);

    if (status == ProfileUpdateStatus::Success) {
        nlohmann::json json;
        response.http_status = 204;
    } else {
        static const std::unordered_map<ProfileUpdateStatus, std::pair<std::string, std::string>> status_map{
            {ProfileUpdateStatus::Failure, {"FF_FAILURE", "Failed to update profile."}},
            {ProfileUpdateStatus::InvalidCreds, {"FF_INVALID_CREDS", "Invalid credentials."}},
            {ProfileUpdateStatus::InvalidIcon, {"FF_INVALID_ICON", "Invalid icon."}},
            {ProfileUpdateStatus::InvalidJson, {"FF_INVALID_JSON", "Invalid JSON."}},
        };
        nlohmann::json json;
        if (status_map.find(status) != status_map.end()) {
            json["error"] = status_map.at(status).first;
            json["error_str"] = status_map.at(status).second;
        } else {
            json["error"] = "FF_UNKNOWN_ERROR";
            json["error_str"] = "Unknown error.";
        }

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
    }

    return response;
}

limhamn::http::server::response ff::handle_api_get_profile(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    if (request.method != "POST") {
        nlohmann::json json;
        json["error"] = "FF_METHOD_NOT_ALLOWED";
        json["error_str"] = "Method not allowed.";
        response.content_type = "application/json";
        response.http_status = 405;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
        nlohmann::json json;
        json["error"] = "FF_NO_BODY";
        json["error_str"] = "No body.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (input.find("usernames") == input.end() || !input.at("usernames").is_array()) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    std::vector<std::string> usernames;
    try {
        usernames = input.at("usernames").get<std::vector<std::string>>();
    } catch (const std::exception&) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json response_json;
    response_json["users"] = nlohmann::json::object();

    if (usernames.empty()) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "No usernames provided.";
        response.content_type = "application/json";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    for (const auto& it : usernames) {
        try {
            const auto _json = nlohmann::json::parse(ff::get_json_from_table(db, "users", "username", it));
            if (!_json.contains("profile") || !_json.at("profile").is_object()) {
                continue;
            }
            const auto& json = _json.at("profile");
            if (json.contains("display_name") && json.at("display_name").is_string()) {
                response_json["users"][it]["display_name"] = json.at("display_name").get<std::string>();
            } else {
                response_json["users"][it]["display_name"] = it;
            }
            if (json.contains("description") && json.at("description").is_string()) {
                response_json["users"][it]["description"] = json.at("description").get<std::string>();
            } else {
                response_json["users"][it]["description"] = "";
            }
            if (json.contains("profile_key") && json.at("profile_key").is_string()) {
                response_json["users"][it]["profile_key"] = json.at("profile_key").get<std::string>();
            } else {
                response_json["users"][it]["profile_key"] = "";
            }
        } catch (const std::exception& e) {
            ff::logger.write_to_log(limhamn::logger::type::error, "Failed to get profile for user: " + it + ". Error: " + e.what() + "\n");
            continue;
        }
    }

    response.content_type = "application/json";
    response.http_status = 200;
    response.body = response_json.dump();

    return response;
}

limhamn::http::server::response ff::handle_api_create_announcement(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (ff::get_user_type(db, username) != ff::UserType::Administrator) {
        nlohmann::json json;
        json["error"] = "FF_NOT_AUTHORIZED";
        json["error_str"] = "You are not authorized to access this endpoint.";
        response.http_status = 403;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "No body.";
        json["error"] = "FF_NO_BODY";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }
    if (request.method != "POST") {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Not a POST request.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Not a POST request.";
        json["error"] = "FF_INVALID_METHOD";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Failed to parse JSON.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid JSON received";
        json["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json announcement;

    const auto convert_to_markdown = [](const std::string& str) -> std::string {
        maddy::Parser parser;
        std::istringstream stream{str};
        return parser.Parse(stream);
    };

    if (input.find("title") != input.end() && input.at("title").is_string()) {
        announcement["title"] = input.at("title").get<std::string>();
    }

    if (input.find("text_markdown") != input.end() && input.at("text_markdown").is_string()) {
        announcement["text_markdown"] = input.at("text_markdown").get<std::string>();
        announcement["text_html"] = convert_to_markdown(input.at("text_markdown").get<std::string>());
    } else {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON received";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (input.find("publish_timestamp") != input.end() && input.at("publish_timestamp").is_number_integer()) {
        announcement["publish_timestamp"] = input.at("publish_timestamp").get<int64_t>();
    } else {
        announcement["publish_timestamp"] = scrypto::return_unix_millis();
    }

    announcement["author"] = username;

    const auto query = db.query("SELECT * FROM general WHERE id=1;");
    if (query.empty()) {
        nlohmann::json json;
        json["error"] = "FF_FUCK_MY_LIFE";
        json["error_str"] = "Shit is broken, don't know why.";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    auto json = query.at(0).at("json");

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(json);
    } catch (const std::exception&) {
        nlohmann::json ret_json;
        ret_json["error"] = "FF_SERVER_ERROR";
        ret_json["error_str"] = "Server failed, no idea why.";
        response.http_status = 400;
        response.body = ret_json.dump();
        return response;
    }

    db_json["announcements"].push_back(announcement);

    db.exec("UPDATE general SET json = ? WHERE id = 1", db_json.dump());

    response.http_status = 201;
    return response;
}

limhamn::http::server::response ff::handle_api_get_announcements(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    const auto query = db.query("SELECT * FROM general WHERE id=1;");
    if (query.empty()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why. general is empty";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (query.at(0).find("json") == query.at(0).end()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    try {
        const auto json = nlohmann::json::parse(query.at(0).at("json"));
        if (json.contains("announcements") && json.at("announcements").is_array()) {
            response.content_type = "application/json";
            response.http_status = 200;
            response.body = json.dump();
        } else {
            nlohmann::json ret;
            ret["error"] = "FF_SERVER_ERROR";
            ret["error_str"] = "Server failed, no idea why.";

            response.content_type = "application/json";
            response.http_status = 400;
            response.body = ret.dump();

            return response;
        }
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();

        return response;
    }

    return response;
}

limhamn::http::server::response ff::handle_api_delete_announcement(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (ff::get_user_type(db, username) != ff::UserType::Administrator) {
        nlohmann::json json;
        json["error"] = "FF_NOT_AUTHORIZED";
        json["error_str"] = "You are not authorized to access this endpoint.";
        response.http_status = 403;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "No body.";
        json["error"] = "FF_NO_BODY";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }
    if (request.method != "POST") {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Not a POST request.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Not a POST request.";
        json["error"] = "FF_INVALID_METHOD";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Failed to parse JSON.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid JSON received";
        json["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (input.find("announcement_id") == input.end() || !input.at("announcement_id").is_number_integer()) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON received";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    const auto query = db.query("SELECT * FROM general WHERE id=1;");
    if (query.empty()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why. general is empty";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (query.at(0).find("json") == query.at(0).end()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    try {
        auto json = nlohmann::json::parse(query.at(0).at("json"));
        if (json.contains("announcements") && json.at("announcements").is_array()) {
            auto& announcements = json["announcements"];
            const int announcement_id = input.at("announcement_id").get<int>();
            if (announcement_id < 0 || announcement_id >= static_cast<int>(announcements.size())) {
                nlohmann::json ret;
                ret["error"] = "FF_INVALID_JSON";
                ret["error_str"] = "Invalid announcement_id.";
                response.http_status = 400;
                response.body = ret.dump();
                return response;
            }
            announcements.erase(announcements.begin() + announcement_id);
        }
        db.exec("UPDATE general SET json = ? WHERE id = 1", json.dump());
        response.http_status = 204;
        return response;
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();

        return response;
    }
}

limhamn::http::server::response ff::handle_api_edit_announcement(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (ff::get_user_type(db, username) != ff::UserType::Administrator) {
        nlohmann::json json;
        json["error"] = "FF_NOT_AUTHORIZED";
        json["error_str"] = "You are not authorized to access this endpoint.";
        response.http_status = 403;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "No body.";
        json["error"] = "FF_NO_BODY";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }
    if (request.method != "POST") {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Not a POST request.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Not a POST request.";
        json["error"] = "FF_INVALID_METHOD";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Failed to parse JSON.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid JSON received";
        json["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (input.find("announcement_id") == input.end() || !input.at("announcement_id").is_number_integer()) {
        nlohmann::json json;
        json["error"] = "FF_INVALID_JSON";
        json["error_str"] = "Invalid JSON received";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    const auto query = db.query("SELECT * FROM general WHERE id=1;");
    if (query.empty()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why. general is empty";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (query.at(0).find("json") == query.at(0).end()) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    try {
        auto json = nlohmann::json::parse(query.at(0).at("json"));
        if (json.contains("announcements") && json.at("announcements").is_array()) {
            auto& announcements = json["announcements"];
            const int announcement_id = input.at("announcement_id").get<int>();
            if (announcement_id < 0 || announcement_id >= static_cast<int>(announcements.size())) {
                nlohmann::json ret;
                ret["error"] = "FF_INVALID_JSON";
                ret["error_str"] = "Invalid announcement_id.";
                response.http_status = 400;
                response.body = ret.dump();
                return response;
            }
            auto& announcement = announcements[announcement_id];
            if (input.find("title") != input.end() && input.at("title").is_string()) {
                announcement["title"] = input.at("title").get<std::string>();
            }
            if (input.find("text_markdown") != input.end() && input.at("text_markdown").is_string()) {
                announcement["text_markdown"] = input.at("text_markdown").get<std::string>();
                maddy::Parser parser;
                std::istringstream stream{input.at("text_markdown").get<std::string>()};
                announcement["text_html"] = parser.Parse(stream);
            }
            if (input.find("text_html") != input.end() && input.at("text_html").is_string()) {
                announcement["text_html"] = input.at("text_html").get<std::string>();
            }
            if (input.find("publish_timestamp") != input.end() && input.at("publish_timestamp").is_number_integer()) {
                announcement["publish_timestamp"] = input.at("publish_timestamp").get<int64_t>();
            }
            announcement["author"] = username;
        }
        db.exec("UPDATE general SET json = ? WHERE id = 1", json.dump());
        response.http_status = 204;
        return response;
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error"] = "FF_SERVER_ERROR";
        ret["error_str"] = "Server failed, no idea why.";

        response.content_type = "application/json";
        response.http_status = 400;
        response.body = ret.dump();

        return response;
    }
}


limhamn::http::server::response ff::handle_api_rate_file_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("file_identifier") || !json.at("file_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("rating") || !json.at("rating").is_number_integer()) {
        nlohmann::json ret;
        ret["error_str"] = "rating is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const int rating = json.at("rating").get<int>();
    if ((rating < 1 || rating > 5) && rating != 0) {
        nlohmann::json ret;
        ret["error_str"] = "rating must be between 1 and 5";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    // rating == 0 means that the user wants to remove their rating for the file
    // rating should replace any existing rating for the file
    const std::string file_identifier = json.at("file_identifier").get<std::string>();
    if (file_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "sandbox", "identifier", file_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    // db_json["ratings"]["username"]["rating"] = rating;
    if (db_json.find("ratings") == db_json.end() || !db_json.at("ratings").is_object()) {
        db_json["ratings"] = nlohmann::json::object();
    }
    if (rating == 0) {
        // remove the rating for the user
        db_json["ratings"].erase(username);
    } else {
        // set the rating for the user
        db_json["ratings"][username]["rating"] = rating;
    }

    ff::set_json_in_table(db, "sandbox", "identifier", file_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_rate_forwarder_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("forwarder_identifier") || !json.at("forwarder_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("rating") || !json.at("rating").is_number_integer()) {
        nlohmann::json ret;
        ret["error_str"] = "rating is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const int rating = json.at("rating").get<int>();
    if ((rating < 1 || rating > 5) && rating != 0) {
        nlohmann::json ret;
        ret["error_str"] = "rating must be between 1 and 5";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    // rating == 0 means that the user wants to remove their rating for the file
    // rating should replace any existing rating for the file
    const std::string forwarder_identifier = json.at("forwarder_identifier").get<std::string>();
    if (forwarder_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", forwarder_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    // db_json["ratings"]["username"]["rating"] = rating;
    if (db_json.find("ratings") == db_json.end() || !db_json.at("ratings").is_object()) {
        db_json["ratings"] = nlohmann::json::object();
    }
    if (rating == 0) {
        // remove the rating for the user
        db_json["ratings"].erase(username);
    } else {
        // set the rating for the user
        db_json["ratings"][username]["rating"] = rating;
    }

    ff::set_json_in_table(db, "forwarders", "identifier", forwarder_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_comment_forwarder_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("forwarder_identifier") || !json.at("forwarder_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("comment_text") || !json.at("comment_text").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_text is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string forwarder_identifier = json.at("forwarder_identifier").get<std::string>();
    if (forwarder_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string comment_text = json.at("comment_text").get<std::string>();
    if (comment_text.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_text is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", forwarder_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Forwarder not found";
        ret["error"] = "FF_FORWARDER_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
    if (db_json.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "Forwarder not found";
        ret["error"] = "FF_FORWARDER_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    // db_json["reviews"]["username"]["comment"] = comment_text;
    if (db_json.find("reviews") == db_json.end() || !db_json.at("reviews").is_array()) {
        db_json["reviews"] = nlohmann::json::array();
    }

    db_json["reviews"].push_back({
        {"comment", limhamn::http::utils::htmlspecialchars(comment_text)},
        {"timestamp", scrypto::return_unix_millis()},
        {"username", username}
    });

    ff::set_json_in_table(db, "forwarders", "identifier", forwarder_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_comment_file_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("file_identifier") || !json.at("file_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("comment_text") || !json.at("comment_text").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_text is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string file_identifier = json.at("file_identifier").get<std::string>();
    if (file_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string comment_text = json.at("comment_text").get<std::string>();
    if (comment_text.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_text is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "sandbox", "identifier", file_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
    if (db_json.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "Forwarder not found";
        ret["error"] = "FF_FORWARDER_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    if (db_json.find("reviews") == db_json.end() || !db_json.at("reviews").is_array()) {
        db_json["reviews"] = nlohmann::json::array();
    }

    db_json["reviews"].push_back({
        {"comment", limhamn::http::utils::htmlspecialchars(comment_text)},
        {"timestamp", scrypto::return_unix_millis()},
        {"username", username}
    });

    ff::set_json_in_table(db, "sandbox", "identifier", file_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_delete_comment_forwarder_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("forwarder_identifier") || !json.at("forwarder_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("comment_identifier") || !json.at("comment_identifier").is_number_integer()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string forwarder_identifier = json.at("forwarder_identifier").get<std::string>();
    if (forwarder_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const int comment_identifier = json.at("comment_identifier").get<int>();

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", forwarder_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
    if (db_json.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "Forwarder not found";
        ret["error"] = "FF_FORWARDER_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    auto& reviews = db_json["reviews"];
    if (reviews.is_array() && comment_identifier >= 0 && comment_identifier < static_cast<int>(reviews.size())) {
        // it must have the same username as the user who is trying to delete the comment OR user_type must be Administrator
        if ((reviews[comment_identifier].find("username") == reviews[comment_identifier].end() ||
            reviews[comment_identifier].at("username").get<std::string>() != username) && get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only delete your own comments";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

        reviews.erase(reviews.begin() + comment_identifier);
    } else {
        nlohmann::json ret;
        ret["error_str"] = "Invalid comment_identifier";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    ff::set_json_in_table(db, "forwarders", "identifier", forwarder_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_delete_comment_file_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("file_identifier") || !json.at("file_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("comment_identifier") || !json.at("comment_identifier").is_number_integer()) {
        nlohmann::json ret;
        ret["error_str"] = "comment_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string file_identifier = json.at("file_identifier").get<std::string>();
    if (file_identifier.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const int comment_identifier = json.at("comment_identifier").get<int>();

    nlohmann::json db_json;
    try {
        db_json = nlohmann::json::parse(ff::get_json_from_table(db, "sandbox", "identifier", file_identifier));
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
    if (db_json.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    auto& reviews = db_json["reviews"];
    if (reviews.is_array() && comment_identifier >= 0 && comment_identifier < static_cast<int>(reviews.size())) {
        // it must have the same username as the user who is trying to delete the comment OR user_type must be Administrator
        if ((reviews[comment_identifier].find("username") == reviews[comment_identifier].end() ||
            reviews[comment_identifier].at("username").get<std::string>() != username) && get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only delete your own comments";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

        reviews.erase(reviews.begin() + comment_identifier);
    } else {
        nlohmann::json ret;
        ret["error_str"] = "Invalid comment_identifier";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    ff::set_json_in_table(db, "sandbox", "identifier", file_identifier, db_json.dump());

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_delete_file_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("file_identifier") || !json.at("file_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "file_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string& file_identifier = json.at("file_identifier").get<std::string>();

    try {
        nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "sandbox", "identifier", file_identifier));
        const auto& uploader = db_json.at("uploader").get<std::string>();
        if (username != uploader && get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only delete your own files";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

        db.exec("DELETE FROM sandbox WHERE identifier = ?", file_identifier);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_delete_forwarder_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (!json.contains("forwarder_identifier") || !json.at("forwarder_identifier").is_string()) {
        nlohmann::json ret;
        ret["error_str"] = "forwarder_identifier is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    const std::string& forwarder_identifier = json.at("forwarder_identifier").get<std::string>();

    try {
        nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", forwarder_identifier));
        const auto& uploader = db_json.at("uploader").get<std::string>();
        if (username != uploader && get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only delete your own files";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

        db.exec("DELETE FROM forwarders WHERE identifier = ?", forwarder_identifier);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "File not found";
        ret["error"] = "FF_FILE_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_stay_logged_in(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    if (request.session_id.empty()) {
        nlohmann::json json;
        json["error"] = "FF_SESSION_NOT_FOUND";
        json["error_str"] = "Session not found.";
        response.body = json.dump();
        response.http_status = 400;
        return response;
    }

    auto now = scrypto::return_unix_millis();
    auto expires = now + (30LL * 24 * 60 * 60 * 1000);

    ff::logger.write_to_log(limhamn::logger::type::notice, "Setting session cookie with name: " + settings.session_cookie_name + ", value: " + request.session_id + ", expires: " + std::to_string(expires) + "\n");

    response.cookies.push_back(limhamn::http::server::cookie{
        .name = settings.session_cookie_name,
        .value = request.session_id,
        .expires = expires,
        .path = "/",
        .same_site = "Strict",
        .http_only = true,
    });
    for (const auto& it : request.cookies) {
        if (it.name == "username" || it.name == "user_type") {
            response.cookies.push_back(limhamn::http::server::cookie{
                .name = it.name,
                .value = it.value,
                .expires = expires,
                .path = "/",
                .same_site = "Strict",
                .http_only = false,
            });
        }
    }

    response.http_status = 204;
    response.body = "";
    response.content_type = "application/json";

    return response;
}

limhamn::http::server::response ff::handle_api_try_logout_endpoint(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};

    response.content_type = "application/json";
    response.http_status = 204;
    response.body = "";
    response.delete_cookies.emplace_back(settings.session_cookie_name);
    response.delete_cookies.emplace_back("username");
    response.delete_cookies.emplace_back("user_type");

    return response;
}

limhamn::http::server::response ff::handle_api_create_topic(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	if (get_user_type(db, username) != ff::UserType::Administrator && settings.topics_require_admin) {
		nlohmann::json ret;
		ret["error_str"] = "You are not allowed to create topics";
		ret["error"] = "FF_NOT_AUTHORIZED";
		response.http_status = 403;
		response.body = ret.dump();
		return response;
	}

    std::string title{};
    std::string description{};
    std::string topic_id = scrypto::generate_random_string(4);

    if (json.contains("title") && json.at("title").is_string()) {
        title = json.at("title").get<std::string>();
    }

    if (json.contains("description") && json.at("description").is_string()) {
        description = json.at("description").get<std::string>();
    }

    if (json.contains("topic_id") && json.at("topic_id").is_string()) {
        topic_id = json.at("topic_id").get<std::string>();
    }

    const auto check_if_topic_exists = [&db, &topic_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

    int i = 4;
    while (check_if_topic_exists()) {
        topic_id = scrypto::generate_random_string(i);
        ++i;
    }

	bool open = true;
	if (json.contains("open") && json.at("open").is_boolean()) {
		open = json.at("open").get<bool>();
	}

    nlohmann::json db_json;

    db_json["title"] = limhamn::http::utils::htmlspecialchars(title);
    db_json["description"] = limhamn::http::utils::htmlspecialchars(description);
    db_json["created_by"] = username;
    db_json["created_at"] = scrypto::return_unix_millis();
    db_json["identifier"] = topic_id;
    db_json["topics"] = nlohmann::json::array(); // {identifier, pinned?}
    db_json["posts"] = nlohmann::json::array(); // {identifier, pinned?}
    db_json["open"] = open;

    try {
        ff::set_json_in_table(db, "topics", "identifier", topic_id, db_json.dump());
    } catch (const std::exception& e) {
        nlohmann::json ret;
        ret["error_str"] = "Failed to create topic: " + std::string(e.what());
        ret["error"] = "FF_DATABASE_ERROR";
        response.http_status = 500;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json ret;
    ret["topic_id"] = topic_id;

    response.http_status = 200;
    response.content_type = "application/json";
    response.body = ret.dump();

    return response;
}

limhamn::http::server::response ff::handle_api_delete_topic(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    std::string topic_id{};
    if (json.contains("topic_id") && json.at("topic_id").is_string()) {
        topic_id = json.at("topic_id").get<std::string>();
    } else {
        nlohmann::json ret;
        ret["error_str"] = "topic_id is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (topic_id.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "topic_id is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	try {
		const auto db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Topic not found";
			ret["error"] = "FF_TOPIC_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
			get_user_type(db, username) != ff::UserType::Administrator) {
			nlohmann::json ret;
			ret["error_str"] = "You can only delete your own topics";
			ret["error"] = "FF_NOT_AUTHORIZED";
			response.http_status = 403;
			response.body = ret.dump();
			return response;
		}
	} catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Topic not found";
        ret["error"] = "FF_TOPIC_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

    try {
        db.exec("DELETE FROM topics WHERE identifier = ?", topic_id);

    	const auto posts = db.query("SELECT * FROM posts;");
    	for (const auto& post : posts) {
    		const auto post_json = nlohmann::json::parse(post.at("json"));

    		if (post_json.contains("topic_id") && post_json.at("topic_id").get<std::string>() == topic_id) {
    			db.exec("DELETE FROM posts WHERE identifier = ?", post.at("identifier"));
    		}
    	}
    } catch (const std::exception& e) {
        nlohmann::json ret;
        ret["error_str"] = "Failed to delete topic: " + std::string(e.what());
        ret["error"] = "FF_DATABASE_ERROR";
        response.http_status = 500;
        response.body = ret.dump();
        return response;
    }

    response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_get_topics(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

	int start_index{};
	int end_index = -1;
	std::vector<std::string> ids{};

    try {
		nlohmann::json input_json = nlohmann::json::parse(request.body);

        if (input_json.contains("ids") && input_json.at("ids").is_array()) {
            for (const auto& it : input_json.at("ids")) {
                if (it.is_string()) {
                    ids.push_back(it.get<std::string>());
                }
            }
        }

        if (input_json.contains("start_index") && input_json.at("start_index").is_number_integer()) {
            start_index = input_json.at("start_index").get<int>();
        }

        if (input_json.contains("end_index") && input_json.at("end_index").is_number_integer()) {
            end_index = input_json.at("end_index").get<int>();
        }
    } catch (const std::exception&) {}

	nlohmann::json json;

	json["topics"] = nlohmann::json::array();

    try {
    	auto contents = db.query("SELECT * FROM topics");

    	int i{};
    	for (const auto& it : contents) {
    		if (i < start_index) {
    			++i;
    			continue;
    		}
    		if (end_index >= 0 && i > end_index) {
    			break;
    		}

    		const auto db_json = nlohmann::json::parse(it.at("json"));

    		if (!db_json.contains("identifier") || !db_json.at("identifier").is_string()) {
    			continue;
    		}

    		if (!ids.empty() && std::find(ids.begin(), ids.end(), db_json.at("identifier").get<std::string>()) == ids.end()) {
    			continue; // skip if the identifier is not in the list of ids
    		}

    		json["topics"].push_back(db_json);
    		++i;
    	}
    } catch (const std::exception& e) {
        nlohmann::json ret;
        ret["error_str"] = "Failed to get topics: " + std::string(e.what());
        ret["error"] = "FF_DATABASE_ERROR";
        response.http_status = 500;
        response.body = ret.dump();
        return response;
    }

    response.http_status = 200;
    response.body = json.dump();
    return response;
}

limhamn::http::server::response ff::handle_api_close_topic(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    std::string topic_id{};
    if (json.contains("topic_id") && json.at("topic_id").is_string()) {
        topic_id = json.at("topic_id").get<std::string>();
    } else {
        nlohmann::json ret;
        ret["error_str"] = "topic_id is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

    if (topic_id.empty()) {
        nlohmann::json ret;
        ret["error_str"] = "topic_id is empty";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	bool open = false;
	if (json.contains("open") && json.at("open").is_boolean()) {
		open = json.at("open").get<bool>();
	}

    try {
        nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
        if (db_json.empty()) {
            nlohmann::json ret;
            ret["error_str"] = "Topic not found";
            ret["error"] = "FF_TOPIC_NOT_FOUND";
            response.http_status = 404;
            response.body = ret.dump();
            return response;
        }

        if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
            get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only close your own topics";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

        db_json["open"] = open;

        ff::set_json_in_table(db, "topics", "identifier", topic_id, db_json.dump());
    } catch (const std::exception&) {
	    nlohmann::json ret;
    	ret["error_str"] = "Topic not found";
    	ret["error"] = "FF_TOPIC_NOT_FOUND";
    	response.http_status = 404;
    	response.body = ret.dump();
    	return response;
    }

	response.http_status = 204;
    response.body = "";
    return response;
}

limhamn::http::server::response ff::handle_api_edit_topic(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
	response.content_type = "application/json";

	const auto get_username = [&request]() -> std::string {
		if (request.session.find("username") != request.session.end()) {
			return request.session.at("username");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("username") != json.end() && json.at("username").is_string()) {
				return json.at("username").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const auto get_key = [&request]() -> std::string {
		if (request.session.find("key") != request.session.end()) {
			return request.session.at("key");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("key") != json.end() && json.at("key").is_string()) {
				return json.at("key").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const std::string username{get_username()};
	const std::string key{get_key()};

	if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Username or key is empty.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Invalid credentials.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(request.body);
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Invalid JSON";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string topic_id{};
	if (json.contains("topic_id") && json.at("topic_id").is_string()) {
		topic_id = json.at("topic_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "topic_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	if (topic_id.empty()) {
		nlohmann::json ret;
		ret["error_str"] = "topic_id is empty";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Topic not found";
			ret["error"] = "FF_TOPIC_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
            get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only edit your own topics";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

		if (json.contains("title") && json.at("title").is_string()) {
            db_json["title"] = limhamn::http::utils::htmlspecialchars(json.at("title").get<std::string>());
        }
		if (json.contains("description") && json.at("description").is_string()) {
            db_json["description"] = limhamn::http::utils::htmlspecialchars(json.at("description").get<std::string>());
        }

		ff::set_json_in_table(db, "topics", "identifier", topic_id, db_json.dump());
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Topic not found";
		ret["error"] = "FF_TOPIC_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}

	response.http_status = 204;
	response.body = "";
	return response;
}

limhamn::http::server::response ff::handle_api_create_post(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	if (get_user_type(db, username) != ff::UserType::Administrator && settings.topics_require_admin) {
		nlohmann::json ret;
		ret["error_str"] = "You are not allowed to create topics";
		ret["error"] = "FF_NOT_AUTHORIZED";
		response.http_status = 403;
		response.body = ret.dump();
		return response;
	}

    std::string title{};
    std::string text{};
    std::string post_id = scrypto::generate_random_string(4);
	std::string topic_id{};

    if (json.contains("title") && json.at("title").is_string()) {
        title = json.at("title").get<std::string>();
    }

    if (json.contains("text") && json.at("text").is_string()) {
        text = json.at("description").get<std::string>();
    }

    if (json.contains("post_id") && json.at("post_id").is_string()) {
        post_id = json.at("post_id").get<std::string>();
    }

	if (json.contains("topic_id") && json.at("topic_id").is_string()) {
		topic_id = json.at("topic_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "topic_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

    const auto check_if_topic_exists = [&db, &topic_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

	if (!check_if_topic_exists()) {
		nlohmann::json ret;
		ret["error_str"] = "Topic not found";
		ret["error"] = "FF_TOPIC_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}

	const auto check_if_post_exists = [&db, &post_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

    int i = 4;
    while (check_if_post_exists()) {
        post_id = scrypto::generate_random_string(i);
        ++i;
    }

	bool open = true;
	if (json.contains("open") && json.at("open").is_boolean()) {
		open = json.at("open").get<bool>();
	}

    nlohmann::json db_json;

    db_json["title"] = limhamn::http::utils::htmlspecialchars(title);
    db_json["text"] = limhamn::http::utils::htmlspecialchars(text);
    db_json["created_by"] = username;
    db_json["created_at"] = scrypto::return_unix_millis();
    db_json["identifier"] = post_id;
    db_json["open"] = open;
	db_json["comments"] = nlohmann::json::array();
    db_json["topic_id"] = topic_id;

    try {
        ff::set_json_in_table(db, "posts", "identifier", post_id, db_json.dump());
    } catch (const std::exception& e) {
        nlohmann::json ret;
        ret["error_str"] = "Failed to create post: " + std::string(e.what());
        ret["error"] = "FF_DATABASE_ERROR";
        response.http_status = 500;
        response.body = ret.dump();
        return response;
    }

    nlohmann::json ret;
	ret["post_id"] = post_id;
    ret["topic_id"] = topic_id;

    response.http_status = 200;
    response.content_type = "application/json";
    response.body = ret.dump();

    return response;
}

limhamn::http::server::response ff::handle_api_delete_post(const limhamn::http::server::request& request, database& db) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("username") != json.end() && json.at("username").is_string()) {
                return json.at("username").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
        }

        try {
            const auto json = nlohmann::json::parse(request.body);
            if (json.find("key") != json.end() && json.at("key").is_string()) {
                return json.at("key").get<std::string>();
            }
        } catch (const std::exception&) {
            // ignore
        }

        return "";
    };

    const std::string username{get_username()};
    const std::string key{get_key()};

    if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Username or key is empty.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error_str"] = "Invalid credentials.";
        json["error"] = "FF_INVALID_CREDENTIALS";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(request.body);
    } catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Invalid JSON";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	if (get_user_type(db, username) != ff::UserType::Administrator && settings.topics_require_admin) {
		nlohmann::json ret;
		ret["error_str"] = "You are not allowed to create topics";
		ret["error"] = "FF_NOT_AUTHORIZED";
		response.http_status = 403;
		response.body = ret.dump();
		return response;
	}

    std::string post_id = scrypto::generate_random_string(4);
    if (json.contains("post_id") && json.at("post_id").is_string()) {
        post_id = json.at("post_id").get<std::string>();
    }

	const auto check_if_post_exists = [&db, &post_id]() -> bool {
        try {
            nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
            return !db_json.empty();
        } catch (const std::exception&) {
            return false;
        }
    };

	if (!check_if_post_exists()) {
        nlohmann::json ret;
        ret["error_str"] = "Post not found";
        ret["error"] = "FF_POST_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));

		if (db_json.empty()) {
            nlohmann::json ret;
            ret["error_str"] = "Post not found";
            ret["error"] = "FF_POST_NOT_FOUND";
            response.http_status = 404;
            response.body = ret.dump();
            return response;
        }

        if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
            get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only delete your own posts";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
            const std::string topic_id = db_json.at("topic_id").get<std::string>();
            nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", topic_id));
            if (topic_json.empty()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic not found";
                ret["error"] = "FF_TOPIC_NOT_FOUND";
                response.http_status = 404;
                response.body = ret.dump();
                return response;
            }

            if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic is closed";
                ret["error"] = "FF_TOPIC_CLOSED";
                response.http_status = 403;
                response.body = ret.dump();
                return response;
            }

			auto& posts = topic_json["posts"];
            posts.erase(std::remove_if(posts.begin(), posts.end(),
                [&post_id](const nlohmann::json& post) {
                    return post.contains("identifier") && post.at("identifier").get<std::string>() == post_id;
                }), posts.end());

            ff::set_json_in_table(db, "topics", "identifier", topic_id, topic_json.dump());
        }

		// now delete the post
        db.exec("DELETE FROM posts WHERE identifier = ?", post_id);

        response.http_status = 204;
        response.body = "";

        return response;
	} catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Post not found";
        ret["error"] = "FF_POST_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
}

limhamn::http::server::response ff::handle_api_close_post(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
	response.content_type = "application/json";

	const auto get_username = [&request]() -> std::string {
		if (request.session.find("username") != request.session.end()) {
			return request.session.at("username");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("username") != json.end() && json.at("username").is_string()) {
				return json.at("username").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const auto get_key = [&request]() -> std::string {
		if (request.session.find("key") != request.session.end()) {
			return request.session.at("key");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("key") != json.end() && json.at("key").is_string()) {
				return json.at("key").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const std::string username{get_username()};
	const std::string key{get_key()};

	if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Username or key is empty.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Invalid credentials.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(request.body);
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Invalid JSON";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string post_id{};
	if (json.contains("post_id") && json.at("post_id").is_string()) {
		post_id = json.at("post_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "post_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	if (post_id.empty()) {
		nlohmann::json ret;
		ret["error_str"] = "post_id is empty";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	bool open = false;
	if (json.contains("open") && json.at("open").is_boolean()) {
        open = json.at("open").get<bool>();
    }

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Post not found";
			ret["error"] = "FF_POST_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
            get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only close your own posts";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
			nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", db_json.at("topic_id").get<std::string>()));
			if (topic_json.empty()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic not found";
                ret["error"] = "FF_TOPIC_NOT_FOUND";
                response.http_status = 404;
                response.body = ret.dump();
                return response;
            }

			if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic is closed";
                ret["error"] = "FF_TOPIC_CLOSED";
                response.http_status = 403;
                response.body = ret.dump();
                return response;
            }
		}

		db_json["open"] = open;

		ff::set_json_in_table(db, "posts", "identifier", post_id, db_json.dump());
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Topic not found";
		ret["error"] = "FF_TOPIC_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}

	response.http_status = 204;
	response.body = "";
	return response;
}

limhamn::http::server::response ff::handle_api_get_posts(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
    response.content_type = "application/json";

	std::vector<std::string> ids{};
	int start_index{};
	int end_index = -1;

    try {
		nlohmann::json input_json = nlohmann::json::parse(request.body);

        if (input_json.contains("ids") && input_json.at("ids").is_array()) {
            for (const auto& it : input_json.at("ids")) {
                if (it.is_string()) {
                    ids.push_back(it.get<std::string>());
                }
            }
        }

        if (input_json.contains("start_index") && input_json.at("start_index").is_number_integer()) {
            start_index = input_json.at("start_index").get<int>();
        }

        if (input_json.contains("end_index") && input_json.at("end_index").is_number_integer()) {
            end_index = input_json.at("end_index").get<int>();
        }
    } catch (const std::exception&) {}

    nlohmann::json json;
	json["posts"] = nlohmann::json::array();

    try {
    	auto contents = db.query("SELECT * FROM posts");

    	int i{};
    	for (const auto& it : contents) {
    		if (i < start_index) {
    			++i;
    			continue;
    		}
    		if (end_index >= 0 && i > end_index) {
    			break;
    		}

    		const auto db_json = nlohmann::json::parse(it.at("json"));

    		if (!db_json.contains("identifier") || !db_json.at("identifier").is_string()) {
    			continue;
    		}

    		if (!ids.empty() && std::find(ids.begin(), ids.end(), db_json.at("identifier").get<std::string>()) == ids.end()) {
    			continue; // skip if the identifier is not in the list of ids
    		}

    		json["posts"].push_back(db_json);
    		++i;
    	}
    } catch (const std::exception& e) {
        nlohmann::json ret;
        ret["error_str"] = "Failed to get posts: " + std::string(e.what());
        ret["error"] = "FF_DATABASE_ERROR";
        response.http_status = 500;
        response.body = ret.dump();
        return response;
    }

    response.http_status = 200;
    response.body = json.dump();
    return response;
}

limhamn::http::server::response ff::handle_api_edit_post(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
	response.content_type = "application/json";

	const auto get_username = [&request]() -> std::string {
		if (request.session.find("username") != request.session.end()) {
			return request.session.at("username");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("username") != json.end() && json.at("username").is_string()) {
				return json.at("username").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const auto get_key = [&request]() -> std::string {
		if (request.session.find("key") != request.session.end()) {
			return request.session.at("key");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("key") != json.end() && json.at("key").is_string()) {
				return json.at("key").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const std::string username{get_username()};
	const std::string key{get_key()};

	if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Username or key is empty.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Invalid credentials.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(request.body);
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Invalid JSON";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string post_id{};
	if (json.contains("post_id") && json.at("post_id").is_string()) {
        post_id = json.at("post_id").get<std::string>();
    } else {
        nlohmann::json ret;
        ret["error_str"] = "post_id is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
		if (db_json.empty()) {
            nlohmann::json ret;
            ret["error_str"] = "Post not found";
            ret["error"] = "FF_POST_NOT_FOUND";
            response.http_status = 404;
            response.body = ret.dump();
            return response;
        }

		if (db_json.find("created_by") != db_json.end() && db_json.at("created_by").get<std::string>() != username &&
            get_user_type(db, username) != ff::UserType::Administrator) {
            nlohmann::json ret;
            ret["error_str"] = "You can only edit your own posts";
            ret["error"] = "FF_NOT_AUTHORIZED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

		if (db_json.find("open") != db_json.end() && !db_json.at("open").get<bool>()) {
            nlohmann::json ret;
            ret["error_str"] = "Post is closed";
            ret["error"] = "FF_POST_CLOSED";
            response.http_status = 403;
            response.body = ret.dump();
            return response;
        }

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
            nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", db_json.at("topic_id").get<std::string>()));
            if (topic_json.empty()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic not found";
                ret["error"] = "FF_TOPIC_NOT_FOUND";
                response.http_status = 404;
                response.body = ret.dump();
                return response;
            }

            if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
                nlohmann::json ret;
                ret["error_str"] = "Topic is closed";
                ret["error"] = "FF_TOPIC_CLOSED";
                response.http_status = 403;
                response.body = ret.dump();
                return response;
            }
        }

		if (json.contains("title") && json.at("title").is_string()) {
            db_json["title"] = limhamn::http::utils::htmlspecialchars(json.at("title").get<std::string>());
        }
		if (json.contains("text") && json.at("text").is_string()) {
            db_json["text"] = limhamn::http::utils::htmlspecialchars(json.at("text").get<std::string>());
        }

		ff::set_json_in_table(db, "posts", "identifier", post_id, db_json.dump());

		nlohmann::json ret;
		ret["post_id"] = post_id;
		ret["topic_id"] = db_json.at("topic_id").get<std::string>();
		response.http_status = 200;
		response.body = ret.dump();
		return response;
	} catch (const std::exception&) {
        nlohmann::json ret;
        ret["error_str"] = "Post not found";
        ret["error"] = "FF_POST_NOT_FOUND";
        response.http_status = 404;
        response.body = ret.dump();
        return response;
    }
}

limhamn::http::server::response ff::handle_api_comment_post(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
	response.content_type = "application/json";

	const auto get_username = [&request]() -> std::string {
		if (request.session.find("username") != request.session.end()) {
			return request.session.at("username");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("username") != json.end() && json.at("username").is_string()) {
				return json.at("username").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const auto get_key = [&request]() -> std::string {
		if (request.session.find("key") != request.session.end()) {
			return request.session.at("key");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("key") != json.end() && json.at("key").is_string()) {
				return json.at("key").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const std::string username{get_username()};
	const std::string key{get_key()};

	if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Username or key is empty.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Invalid credentials.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(request.body);
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Invalid JSON";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string post_id{};
	if (json.contains("post_id") && json.at("post_id").is_string()) {
		post_id = json.at("post_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "post_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string comment{};
	if (json.contains("comment") && json.at("comment").is_string()) {
        comment = json.at("comment").get<std::string>();
    } else {
        nlohmann::json ret;
        ret["error_str"] = "comment is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Post not found";
			ret["error"] = "FF_POST_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("open") != db_json.end() && !db_json.at("open").get<bool>()) {
			nlohmann::json ret;
			ret["error_str"] = "Post is closed";
			ret["error"] = "FF_POST_CLOSED";
			response.http_status = 403;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
			nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", db_json.at("topic_id").get<std::string>()));
			if (topic_json.empty()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic not found";
				ret["error"] = "FF_TOPIC_NOT_FOUND";
				response.http_status = 404;
				response.body = ret.dump();
				return response;
			}

			if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic is closed";
				ret["error"] = "FF_TOPIC_CLOSED";
				response.http_status = 403;
				response.body = ret.dump();
				return response;
			}
		}

		if (db_json.find("comments") == db_json.end() || !db_json.at("comments").is_array()) {
            db_json["comments"] = nlohmann::json::array();
        }

		nlohmann::json comment_json;
		comment_json["comment"] = limhamn::http::utils::htmlspecialchars(comment);
		comment_json["created_by"] = username;
		comment_json["created_at"] = scrypto::return_unix_millis();

		db_json["comments"].push_back(comment_json);

		ff::set_json_in_table(db, "posts", "identifier", post_id, db_json.dump());

		nlohmann::json ret;
		ret["post_id"] = post_id;
		ret["topic_id"] = db_json.at("topic_id").get<std::string>();
		response.http_status = 200;
		response.body = ret.dump();
		return response;
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Post not found";
		ret["error"] = "FF_POST_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}
}

limhamn::http::server::response ff::handle_api_delete_comment_post(const limhamn::http::server::request& request, database& db) {
	limhamn::http::server::response response{};
	response.content_type = "application/json";

	const auto get_username = [&request]() -> std::string {
		if (request.session.find("username") != request.session.end()) {
			return request.session.at("username");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("username") != json.end() && json.at("username").is_string()) {
				return json.at("username").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const auto get_key = [&request]() -> std::string {
		if (request.session.find("key") != request.session.end()) {
			return request.session.at("key");
		}

		try {
			const auto json = nlohmann::json::parse(request.body);
			if (json.find("key") != json.end() && json.at("key").is_string()) {
				return json.at("key").get<std::string>();
			}
		} catch (const std::exception&) {
			// ignore
		}

		return "";
	};

	const std::string username{get_username()};
	const std::string key{get_key()};

	if (username.empty() || key.empty()) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Username or key is empty.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Username or key is empty.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
		logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
		nlohmann::json json;
		json["error_str"] = "Invalid credentials.";
		json["error"] = "FF_INVALID_CREDENTIALS";
		response.http_status = 400;
		response.body = json.dump();
		return response;
	}

	nlohmann::json json;
	try {
		json = nlohmann::json::parse(request.body);
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Invalid JSON";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	std::string post_id{};
	if (json.contains("post_id") && json.at("post_id").is_string()) {
		post_id = json.at("post_id").get<std::string>();
	} else {
		nlohmann::json ret;
		ret["error_str"] = "post_id is required";
		ret["error"] = "FF_INVALID_JSON";
		response.http_status = 400;
		response.body = ret.dump();
		return response;
	}

	int comment_id{};
	if (json.contains("comment_id") && json.at("comment_id").is_number_integer()) {
        comment_id = json.at("comment_id").get<int>();
    } else {
        nlohmann::json ret;
        ret["error_str"] = "comment_id is required";
        ret["error"] = "FF_INVALID_JSON";
        response.http_status = 400;
        response.body = ret.dump();
        return response;
    }

	try {
		nlohmann::json db_json = nlohmann::json::parse(ff::get_json_from_table(db, "posts", "identifier", post_id));
		if (db_json.empty()) {
			nlohmann::json ret;
			ret["error_str"] = "Post not found";
			ret["error"] = "FF_POST_NOT_FOUND";
			response.http_status = 404;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("open") != db_json.end() && !db_json.at("open").get<bool>()) {
			nlohmann::json ret;
			ret["error_str"] = "Post is closed";
			ret["error"] = "FF_POST_CLOSED";
			response.http_status = 403;
			response.body = ret.dump();
			return response;
		}

		if (db_json.find("topic_id") != db_json.end() && db_json.at("topic_id").is_string()) {
			nlohmann::json topic_json = nlohmann::json::parse(ff::get_json_from_table(db, "topics", "identifier", db_json.at("topic_id").get<std::string>()));
			if (topic_json.empty()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic not found";
				ret["error"] = "FF_TOPIC_NOT_FOUND";
				response.http_status = 404;
				response.body = ret.dump();
				return response;
			}

			if (topic_json.find("open") != topic_json.end() && !topic_json.at("open").get<bool>()) {
				nlohmann::json ret;
				ret["error_str"] = "Topic is closed";
				ret["error"] = "FF_TOPIC_CLOSED";
				response.http_status = 403;
				response.body = ret.dump();
				return response;
			}
		}

		if (db_json.find("comments") != db_json.end() && db_json.at("comments").is_array()) {
            auto& comments = db_json["comments"];
            auto it = std::remove_if(comments.begin(), comments.end(),
                [&username, &db, comment_id](const nlohmann::json& comment) {
                	// must be our comment, or we must be an admin
                    return comment.contains("id") && comment.at("id").get<int>() == comment_id &&
                           (comment.contains("created_by") && comment.at("created_by").get<std::string>() != username &&
                            get_user_type(db, username) != ff::UserType::Administrator);
                });
            if (it != comments.end()) {
                comments.erase(it, comments.end());
            } else {
                nlohmann::json ret;
                ret["error_str"] = "Comment not found";
                ret["error"] = "FF_COMMENT_NOT_FOUND";
                response.http_status = 404;
                response.body = ret.dump();
                return response;
            }
        } else {
            nlohmann::json ret;
            ret["error_str"] = "No comments found";
            ret["error"] = "FF_NO_COMMENTS";
            response.http_status = 404;
            response.body = ret.dump();
            return response;
        }

		nlohmann::json ret;
		response.http_status = 204;
		response.body = "";
		return response;
	} catch (const std::exception&) {
		nlohmann::json ret;
		ret["error_str"] = "Post not found";
		ret["error"] = "FF_POST_NOT_FOUND";
		response.http_status = 404;
		response.body = ret.dump();
		return response;
	}
}