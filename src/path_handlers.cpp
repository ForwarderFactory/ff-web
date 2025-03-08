#include <filesystem>
#include <ff.hpp>
#include <scrypto.hpp>
#include <bygg/bygg.hpp>
#include <nlohmann/json.hpp>

limhamn::http::server::response ff::handle_root_endpoint(const limhamn::http::server::request& request, Database& db, const SiteProperties& site_properties) {
    using namespace bygg::HTML;
    limhamn::http::server::response response{};

    response.content_type = "text/html";
    response.http_status = 200;
    Section sect{Tag::Html, Property{"lang", "en"},
        ff::get_site_head(request, site_properties),
        Section{Tag::Body,
            Element{Tag::H1, make_properties(Property{"id", "page-header"}, Property{"class", "center"}), "Forwarder Factory"},
            Section{Tag::Div, make_properties(Property{"class", "content"}, Property{"id", "content"})}
        }
    };

    static constexpr int body_index = 1; /* deduced from looking at the structure; maybe it's better to use lookup functions? */
    static constexpr int content_index = 1; /* same as above */
    auto& ref = sect.at_section(body_index).at_section(content_index);

    std::vector<Section> sl;
    sl.push_back(ff::get_link_box({.title = "Browse", .description = "Browse channels uploaded by others.", .location = "", .color = "", .background_color = "", .id = "browse-button", .classes = "", .onclick = "show_browse_button()"}));
    if (!ff::username_is_stored(request)) {
        sl.push_back(ff::get_link_box({.title = "Login", .description = "Login to your account.", .location = "", .color = "", .background_color = "", .id = "login-button", .classes = "", .onclick = "show_login()"}));
        if (settings.public_registration) {
            sl.push_back(ff::get_link_box({.title = "Register", .description = "Register for an account.", .location = "", .color = "", .background_color = "", .id = "register-button", .classes = "", .onclick = "show_register()"}));
        }
    } else {
        sl.push_back(ff::get_link_box({.title = "Upload", .description = "Upload a channel to share with others.", .location = "", .color = "", .background_color = "", .id = "upload-button", .classes = "", .onclick = "show_upload()"}));
        sl.push_back(ff::get_link_box({.title = "Log out", .description = "Log out of your account.", .location = "", .color = "", .background_color = "", .id = "logout-button", .classes = "", .onclick = "show_logout_button()"}));
    }
    sl.push_back(ff::get_link_box({.title = "Credits", .description = "See the Forwarder Factory credits.", .location = "", .color = "", .background_color = "", .id = "credits-button", .classes = "", .onclick = "show_credits_button()"}));

    const auto username = request.session.find("username") != request.session.end() ? request.session.at("username") : "";
    if (!username.empty()) {
        const auto type = get_user_type(db, username);
        if (type == UserType::Administrator) {
            sl.push_back(ff::get_link_box({.title = "Administration", .description = "Administer Forwarder Factory, like the God you are.", .location = "", .color = "", .background_color = "", .id = "admin-button", .classes = "", .onclick = "show_admin_button()"}));
        }
    }

    ref.push_back(ff::get_grid(sl, "", "initial-link-grid"));
    response.body = Document(sect).get<std::string>(
#if FF_DEBUG
        Formatting::Pretty
#else
        Formatting::None
#endif
        );

    return response;
}

limhamn::http::server::response ff::handle_api_try_upload_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
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

    const std::pair<ff::UploadStatus, std::string> status = ff::try_upload(request, db);

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

limhamn::http::server::response ff::handle_setup_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};

    if (!ff::needs_setup) {
        response.location = "/";
        return response;
    }

    response.content_type = "text/html";
    response.http_status = 200;

    using namespace bygg::HTML;

    Section sect{Tag::Html, Property{"lang", "en"},
        ff::get_site_head(request, site_properties),
        Section{Tag::Body,
            Element{Tag::H1, make_properties(Property{"id", "page-header"}, Property{"class", "center"}), "Forwarder Factory"},
            Section{Tag::Div, make_properties(Property{"class", "content"}, Property{"id", "content"})},
            Element{Tag::Script, "setup()"},
        }
    };

    response.body = Document(sect).get<std::string>(
#if FF_DEBUG
    Formatting::Pretty
#else
    Formatting::None
#endif
    );

    return response;
}

limhamn::http::server::response ff::handle_try_setup_endpoint(const limhamn::http::server::request& request, Database& db,
    const ff::SiteProperties& site_properties) {
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
    } catch (const std::exception& e) {
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

    return response;
}

limhamn::http::server::response ff::handle_virtual_logo_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};

    response.content_type = "image/svg+xml";
    response.http_status = 200;

    if (settings.logo_file.empty() || !std::filesystem::exists(settings.logo_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    response.body = ff::cache_manager.open_file(settings.logo_file);

    return response;
}

limhamn::http::server::response ff::handle_virtual_favicon_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};

    response.content_type = "image/svg+xml";
    response.http_status = 200;

    if (settings.favicon_file.empty() || !std::filesystem::exists(settings.favicon_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    response.body = ff::cache_manager.open_file(settings.favicon_file);

    return response;
}

limhamn::http::server::response ff::handle_virtual_stylesheet_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};

    response.content_type = "text/css";
    response.http_status = 200;

    if (settings.css_file.empty() || !std::filesystem::exists(settings.css_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    // TODO: Just like the name, this function is UGLY AS FUCK, and does not belong anywhere near
    // a project like this. But I simply cannot be bothered to write a CSS minifier myself, nor
    // am I aware of any C++ library for doing such a thing, and I am therefore just going to call uglifyjs.
    const auto uglifyFile = [](const std::string& path) -> std::string {
        static const std::string temp_file = settings.temp_directory + "/ff_temp.css";
        if (std::filesystem::exists(temp_file)) {
            return temp_file;
        }
        if (std::system("which uglifyjs > /dev/null") != 0) {
            return path;
        }

        std::filesystem::copy_file(path, temp_file);

        // run uglifyjs on the file
        std::string command = "uglifyjs " + temp_file + " -o " + temp_file;
        std::system(command.c_str());

        return temp_file;
    };

#if FF_DEBUG
    response.body = ff::cache_manager.open_file(settings.css_file);
#else
    std::string path = uglifyFile(settings.css_file);
    response.body = ff::cache_manager.open_file(path);
#endif

    return response;
}

limhamn::http::server::response ff::handle_virtual_script_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response;

    response.content_type = "text/javascript";
    response.http_status = 200;

    if (settings.script_file.empty() || !std::filesystem::exists(settings.script_file)) {
        response.http_status = 200;
        response.body = "";
        return response;
    }

    // TODO: Just like the name, this function is UGLY AS FUCK, and does not belong anywhere near
    // a project like this. But I simply cannot be bothered to write a JS minifier myself, nor
    // am I aware of any C++ library for doing such a thing, and I am therefore just going to call uglifyjs.
    const auto uglifyFile = [](const std::string& path) -> std::string {
        static const std::string temp_file = settings.temp_directory + "/ff_temp.js";
        if (std::filesystem::exists(temp_file)) {
            return temp_file;
        }
        if (std::system("which uglifyjs > /dev/null") != 0) {
            return path;
        }

        std::filesystem::copy_file(path, temp_file);

        // run uglifyjs on the file
        std::string command = "uglifyjs " + temp_file + " -o " + temp_file;
        std::system(command.c_str());

        return temp_file;
    };

#if FF_DEBUG
    response.body = ff::cache_manager.open_file(settings.script_file);
#else
    std::string path = uglifyFile(settings.script_file);
    response.body = ff::cache_manager.open_file(path);
#endif

    return response;
}

limhamn::http::server::response ff::handle_api_try_register_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
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
    } catch (const std::exception& e) {
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

limhamn::http::server::response ff::handle_api_try_login_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
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
    } catch (const std::exception& e) {
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
        } catch (const std::exception& e) {
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

limhamn::http::server::response ff::handle_api_get_uploads_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};

    response.content_type = "application/json";
    response.http_status = 200;

    // find filter in the json
    struct Filter {
        bool is_forwarder{false}; // if true, must be a forwarder
        bool accepted{false}; // if true, must be accepted
        bool needs_review{false}; // if true, must need review
        std::string search_string{""}; // if not empty, must contain this string somewhere
        std::string title_id_string{""}; // if not empty, must match this title id
        std::string uploader{""}; // if not empty, must match this uploader
        std::string author{""}; // if not empty, must match this author
        int type{-1}; // if not -1, must match this type (1 = channel, 0 = forwarder)
        std::string category{""}; // if not empty, must match this category
        std::vector<std::string> categories{}; // if not empty, must match one of these categories
        std::string location{""}; // if not empty, must match this location
        int64_t submitted_before{-1}; // if not -1, must be submitted before this time
        int64_t submitted_after{-1}; // if not -1, must be submitted after this time
        std::pair<int64_t, int64_t> submitted_between{-1, -1}; // if not -1, must be submitted between these times
        int vwii{-1}; // if not -1, must match this vwii (0 = unknown, 1 = yes, 2 = no)
        int begin{-1}; // if not -1, start at this index
        int end{-1}; // if not -1, end at this index
        std::string identifier{""}; // if not empty, must match this identifier
    };

    Filter filter{};

    if (request.method == "POST" && !request.body.empty()) {
        nlohmann::json input_json;

        try {
            input_json = nlohmann::json::parse(request.body);
        } catch (const std::exception& e) {
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
            // compatibility
            if (input_json.at("filter").find("category") != input_json.at("filter").end() && input_json.at("filter").at("category").is_string()) {
                filter.category = input_json.at("filter").at("category").get<std::string>();
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
                if (filter.vwii != 0 && filter.vwii != 1 && filter.vwii != 2) {
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
            } catch (const std::exception& e) {
                continue;
            }

            if (forwarders_json.find("meta") == forwarders_json.end()) {
                continue;
            }

            nlohmann::json meta = forwarders_json.at("meta");

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
                for (const auto& category : filter.categories) {
                    if (meta.find("categories") != meta.end() && meta.at("categories").is_array()) {
                        bool found = false;
                        for (const auto& it : meta.at("categories")) {
                            if (it.is_string()) {
                                std::string cat{it.get<std::string>()};
                                std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
                                if (cat == category) {
                                    found = true;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            continue;
                        }
                    }
                }
            }
            if (!filter.category.empty()) {
                if (meta.find("category") != meta.end() && meta.at("category").is_string()) {
                    std::transform(filter.category.begin(), filter.category.end(), filter.category.begin(), ::tolower);
                    std::string category{meta.at("category").get<std::string>()};
                    std::transform(category.begin(), category.end(), category.begin(), ::tolower);
                    if (category != filter.category) {
                        continue;
                    }
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
                if (meta.find("vwii_compatible") != meta.end() && meta.at("vwii_compatible").is_number_integer()) {
                    if (meta.at("vwii_compatible").get<int>() != filter.vwii) {
                        continue;
                    }
                }
            }

            json["forwarders"].push_back(forwarders_json);
            ++i;
        }
    };

    get_forwarders();

    response.body = json.dump();

    return response;
}

// this endpoint requires auth and cookies
// in the future, we should allow other kinds of auth for this endpoint, so that third party clients can use it
limhamn::http::server::response ff::handle_api_set_approval_for_uploads_endpoint(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
    limhamn::http::server::response response{};
    response.content_type = "application/json";

    if (!ff::username_is_stored(request)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "User is not logged in.\n");
#endif
        nlohmann::json json;
        json["error"] = "User is not logged in.";
        response.body = json.dump();
        response.http_status = 400;
        return response;
    }

    const auto get_username = [&request]() -> std::string {
        if (request.session.find("username") != request.session.end()) {
            return request.session.at("username");
        }

        return "";
    };

    const auto get_key = [&request]() -> std::string {
        if (request.session.find("key") != request.session.end()) {
            return request.session.at("key");
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
        json["error"] = "Username or key is empty.";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (!ff::verify_key(db, username, key)) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Invalid credentials.\n");
#endif
        nlohmann::json json;
        json["error"] = "Invalid credentials.";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    if (request.body.empty()) {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "No body.\n");
#endif
        nlohmann::json json;
        json["error"] = "No body.";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }
    if (request.method != "POST") {
#ifdef FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Not a POST request.\n");
#endif
        nlohmann::json json;
        json["error"] = "Not a POST request.";
        response.http_status = 400;
        response.body = json.dump();
        return response;
    }

    nlohmann::json input;
    try {
        input = nlohmann::json::parse(request.body);
    } catch (const std::exception& e) {
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

    if (input.find("forwarders") == input.end() || !input.at("forwarders").is_object()) {
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

    nlohmann::json forwarders = input.at("forwarders");
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
                nlohmann::json json = nlohmann::json::parse(ff::get_json_from_table(db, "forwarders", "identifier", identifier));
                json["needs_review"] = false;
                ff::set_json_in_table(db, "forwarders", "identifier", identifier, json.dump());
            } else {
                db.exec("DELETE FROM forwarders WHERE identifier = ?;", identifier);
            }

            // return 204;
            response.http_status = 204;
            return response;
        } catch (const std::exception& e) {
#if FF_DEBUG
            logger.write_to_log(limhamn::logger::type::error, "Help I got caught.\n");
#endif
            continue;
        }
    }

    response.http_status = 400;
    nlohmann::json json;
    json["error_str"] = "Invalid JSON received";
    json["error"] = "FF_INVALID_JSON";
    response.body = json.dump();
    return response;
}

limhamn::http::server::response ff::handle_api_update_profile(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& site_properties) {
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

limhamn::http::server::response ff::handle_api_get_profile(const limhamn::http::server::request& request, Database& db, const ff::SiteProperties& prop) {
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
    } catch (const std::exception& e) {
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

    nlohmann::json usernames = input.at("usernames");
    nlohmann::json response_json;
    response_json["users"] = nlohmann::json::array();
    for (const auto& it : usernames) {
        if (!it.is_string()) {
            nlohmann::json json;
            json["error"] = "FF_INVALID_JSON";
            json["error_str"] = "Invalid JSON.";
            response.content_type = "application/json";
            response.http_status = 400;
            response.body = json.dump();
            return response;
        }

        try {
            const auto json = nlohmann::json::parse(ff::get_json_from_table(db, "users", "username", it.get<std::string>()));
            if (json.find("profile") == json.end() || !json.at("profile").is_object()) {
                continue;
            }

            response_json["users"][it.get<std::string>()] = json.at("profile");
        } catch (const std::exception& e) {
            continue;
        }
    }

    response.content_type = "application/json";
    response.http_status = 200;
    response.body = response_json.dump();

    return response;
}