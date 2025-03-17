#ifndef FF_VERSION
#define FF_VERSION "Undefined"
#endif

#include <algorithm>
#include <sstream>
#include <filesystem>
#include <ff.hpp>
#include <scrypto.hpp>
#include <nlohmann/json.hpp>
#include <limhamn/http/http_utils.hpp>

void ff::print_help(const bool stream) {
    std::stringstream ss;

    ss << "ff-web [options]" << "\n";
    ss << "  -p, --port               Specify the port number to run ff-web on" << "\n";
    ss << "  -c, --config-file        Specify the configuration file to use" << "\n";
    ss << "  -gc, --generate-config   Generate a default configuration file" << "\n";
    ss << "  -he, --halt-on-error     Halt the server on error" << "\n";
    ss << "  -nhe, --no-halt-on-error Do not halt the server on error" << "\n";
    ss << "  -h, --help               Display help information" << "\n";
    ss << "  -v, --version            Display the version number" << "\n";

    stream ? std::cout << ss.str() : std::cerr << ss.str();
}

void ff::print_version(const bool stream) {
    std::stringstream ss;

    ss << "Version: " << FF_VERSION << "\n";

    stream ? std::cout << ss.str() : std::cerr << ss.str();
}

std::string ff::open_file(const std::string& file_path) {
    std::ifstream file{file_path};
    std::string content{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
    file.close();
    return content;
}

void ff::prepare_wd() {
    const auto log_error = [](const std::string& error_msg) {
        ff::logger.write_to_log(limhamn::logger::type::error, error_msg);
        std::exit(EXIT_FAILURE);
    };
    const auto create_directory = [](const std::string& path) -> bool {
        try {
            if (path == "." || path == "..") return true;
            if (std::filesystem::is_directory(path)) return true;
            if (!std::filesystem::create_directories(path)) {
                return false;
            }

            return true;
        } catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    };
    const auto check_if_exists = [](const std::string& path) -> bool {
        return std::filesystem::exists(path);
    };
    const auto remove_all_in_directory = [&check_if_exists](const std::string& path) -> void {
        if (!check_if_exists(path)) {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
#if FF_DEBUG
            ff::logger.write_to_log(limhamn::logger::type::notice, "Removing: " + entry.path().string() + "\n");
#endif
            std::filesystem::remove(entry.path());
        }
    };

    if (!check_if_exists(ff::settings.session_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The session directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.session_directory)) {
            log_error("Failed to create the session directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The session directory was created.\n");
    }

    if (!check_if_exists(ff::settings.data_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The data directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.data_directory)) {
            log_error("Failed to create the data directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The data directory was created.\n");
    }

    if (!check_if_exists(ff::settings.temp_directory)) {
        ff::logger.write_to_log(limhamn::logger::type::notice, "The temp directory does not exist. Creating it.\n");
        if (!create_directory(ff::settings.temp_directory)) {
            log_error("Failed to create the temp directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The temp directory was created.\n");
    }

    if (!check_if_exists(ff::settings.css_file)) {
        std::filesystem::path css_file_path{ff::settings.css_file};
        std::filesystem::path css_file_directory{css_file_path.parent_path()};

        ff::logger.write_to_log(limhamn::logger::type::notice, "The CSS file directory does not exist. Creating it.\n");
        if (!create_directory(css_file_directory)) {
            log_error("Failed to create the CSS file directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The CSS file directory was created.\n");
        ff::logger.write_to_log(limhamn::logger::type::warning, "The CSS file does not exist. Creating a blank file. ff-web will not operate correctly without a stylesheet!!\n");

        std::ofstream css_file{ff::settings.css_file};
        if (css_file.is_open() == false) {
            ff::logger.write_to_log(limhamn::logger::type::warning, "CSS file could not be opened.\n");
        } else {
            css_file << "/* ff-web CSS File */\n\n/* Add your CSS here. */\n";
            css_file.close();
        }
    }

    if (!check_if_exists(ff::settings.html_file)) {
        std::filesystem::path html_file_path{ff::settings.html_file};
        std::filesystem::path html_file_directory{html_file_path.parent_path()};

        ff::logger.write_to_log(limhamn::logger::type::notice, "The HTML file directory does not exist. Creating it.\n");
        if (!create_directory(html_file_directory)) {
            log_error("Failed to create the HTML file directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The HTML file directory was created.\n");
        ff::logger.write_to_log(limhamn::logger::type::warning, "The HTML file does not exist. Creating a blank file. ff-web will not operate correctly without an HTML file!!\n");

        std::ofstream html_file{ff::settings.html_file};
        if (html_file.is_open() == false) {
            ff::logger.write_to_log(limhamn::logger::type::warning, "HTML file could not be opened.\n");
        } else {
            html_file << "The sysadmin responsible for this site has not set up an index file. If you intended to use this instance solely for the API, you can ignore this message.\n";
            html_file.close();
        }
    }

    if (!check_if_exists(ff::settings.script_file)) {
        std::filesystem::path script_file_path{ff::settings.script_file};
        std::filesystem::path script_file_directory{script_file_path.parent_path()};

        ff::logger.write_to_log(limhamn::logger::type::notice, "The JS file directory does not exist. Creating it.\n");
        if (!create_directory(script_file_directory)) {
            log_error("Failed to create the JS file directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The JS file directory was created.\n");
        ff::logger.write_to_log(limhamn::logger::type::warning, "The JS file does not exist. Creating a blank file. ff-web will not operate correctly without a stylesheet!!\n");

        std::ofstream script_file{ff::settings.script_file};
        if (script_file.is_open() == false) {
            ff::logger.write_to_log(limhamn::logger::type::warning, "JS file could not be opened.\n");
        } else {
            script_file << "/* ff-web JS File */\n\n/* Add your JS here. */\n";
            script_file.close();
        }
    }

    if (!check_if_exists(ff::settings.sqlite_database_file) && !ff::settings.enabled_database) {
        std::filesystem::path database_file_path{ff::settings.sqlite_database_file};
        std::filesystem::path database_file_directory{database_file_path.parent_path()};

        ff::logger.write_to_log(limhamn::logger::type::notice, "The database file directory does not exist. Creating it.\n");
        if (!create_directory(database_file_directory)) {
            log_error("Failed to create the database file directory. Do I have adequate permissions? Unrecoverable error.\n");
        }
        ff::logger.write_to_log(limhamn::logger::type::notice, "The database file directory was created.\n");
    }

    if (!check_if_exists(ff::settings.favicon_file) && !ff::settings.favicon_file.empty()) {
        std::filesystem::path favicon_file_path{ff::settings.favicon_file};
        std::filesystem::path favicon_file_directory{favicon_file_path.parent_path()};

        if (!check_if_exists(favicon_file_directory)) {
            ff::logger.write_to_log(limhamn::logger::type::notice, "The favicon file directory does not exist. Creating it.\n");

            if (!create_directory(favicon_file_directory)) {
                ff::logger.write_to_log(limhamn::logger::type::error, "Failed to create the favicon file directory. Do I have adequate permissions? Unrecoverable error.\n");
                std::exit(EXIT_FAILURE);
            }
        }

        ff::logger.write_to_log(limhamn::logger::type::warning, "The favicon file does not exist.\n");
    }

    remove_all_in_directory(ff::settings.temp_directory);
    remove_all_in_directory(ff::settings.session_directory);
}

void ff::start_server() {
    try {
#ifdef FF_ENABLE_SQLITE
#ifndef FF_ENABLE_POSTGRESQL
        settings.enabled_database = false;
#endif
#endif

#ifdef FF_ENABLE_POSTGRESQL
#ifndef FF_ENABLE_SQLITE
        settings.enabled_database = true;
#endif
#endif

#if FF_DEBUG
        logger.write_to_log(limhamn::logger::type::notice, "Using database type: " + std::string(settings.enabled_database ? "PostgreSQL" : "SQLite") + "\n");
#endif

        std::shared_ptr<ff::Database> database = std::make_shared<ff::Database>(settings.enabled_database);

        if (settings.enabled_database) {
#ifdef FF_ENABLE_POSTGRESQL
            database->get_postgres().open(settings.psql_host,
                settings.psql_username,
                settings.psql_password,
                settings.psql_database,
                settings.psql_port);
#endif
        } else {
#ifdef FF_ENABLE_SQLITE
            database->get_sqlite().open(settings.sqlite_database_file);
#endif
        }

        if (!database->good()) {
            ff::fatal = true;
            throw std::runtime_error{"Error opening the database file."};
        }

        setup_database(*database);

        if (!ff::ensure_admin_account_exists(*database)) {
            ff::needs_setup = true;
        }

        limhamn::http::server::server(limhamn::http::server::server_settings{
            .port = settings.port,
            .enable_session = true,
            .session_directory = settings.session_directory,
            .session_cookie_name = settings.session_cookie_name,
            .max_request_size = settings.max_request_size,
            .rate_limits = {},
            .blacklisted_ips = settings.blacklisted_ips,
            .whitelisted_ips = settings.whitelisted_ips,
            .default_rate_limit = settings.rate_limit,
            .trust_x_forwarded_for = settings.trust_x_forwarded_for,
            }, [&](const limhamn::http::server::request& request) -> limhamn::http::server::response {
            ff::logger.write_to_log(limhamn::logger::type::access, "Request received from " + request.ip_address + " to " + request.endpoint + " received, handling it.\n");

            const std::unordered_map<std::string, std::function<limhamn::http::server::response(const limhamn::http::server::request&, Database&)>> handlers{
                {virtual_favicon_path, ff::handle_virtual_favicon_endpoint},
                {virtual_stylesheet_path, ff::handle_virtual_stylesheet_endpoint},
                {virtual_script_path, ff::handle_virtual_script_endpoint},

                {"/", ff::handle_root_endpoint},
                {"/browse", ff::handle_root_endpoint},
                {"/view", ff::handle_root_endpoint},
                {"/upload", ff::handle_root_endpoint},
                {"/login", ff::handle_root_endpoint},
                {"/register", ff::handle_root_endpoint},
                {"/admin", ff::handle_root_endpoint},
                {"/try_setup", ff::handle_try_setup_endpoint},

                {"/api/try_upload", ff::handle_api_try_upload_endpoint},
                {"/api/try_login", ff::handle_api_try_login_endpoint},
                {"/api/try_register", ff::handle_api_try_register_endpoint},
                {"/api/get_uploads", ff::handle_api_get_uploads_endpoint},
                {"/api/set_approval_for_uploads", ff::handle_api_set_approval_for_uploads_endpoint},
                {"/api/update_profile", ff::handle_api_update_profile},
                {"/api/get_profile", ff::handle_api_get_profile},
            };
            const std::unordered_map<std::string, std::function<limhamn::http::server::response(const limhamn::http::server::request&, Database&)>> setup_handlers{
                {virtual_favicon_path, ff::handle_virtual_favicon_endpoint},
                {virtual_stylesheet_path, ff::handle_virtual_stylesheet_endpoint},
                {virtual_script_path, ff::handle_virtual_script_endpoint},
                {"/try_setup", ff::handle_try_setup_endpoint},
                {"/setup", ff::handle_setup_endpoint}
            };

            // handle custom paths
            for (const auto& it : ff::settings.custom_paths) {
                if (it.first == request.endpoint) {
                    limhamn::http::server::response response{};

                    if (!ff::static_exists.is_file(it.second)) {
                        response.content_type = "text/html";
                        response.http_status = 404;
                        response.body = "<p>404 Not Found</p>";

                        return response;
                    }

                    response.body = ff::cache_manager.open_file(it.second);
                    response.http_status = 200;
                    response.content_type = limhamn::http::utils::get_appropriate_content_type(it.first);

                    return response;
                }
            }

            if (needs_setup && setup_handlers.find(request.endpoint) != setup_handlers.end()) {
                return setup_handlers.at(request.endpoint)(request, *database);
            } else if (needs_setup) {
                return setup_handlers.at("/setup")(request, *database);
            }

            if (handlers.find(request.endpoint) != handlers.end()) {
                return handlers.at(request.endpoint)(request, *database);
            }

            // check if a file upload exists and if so, download and serve
            std::string file = request.endpoint;
            if (file.find("/download/") != std::string::npos) {
                std::filesystem::path file_path = file.substr(10); // remove /download/
                file_path = file_path.lexically_normal(); // normalize the path

                if (ff::is_file(*database, file_path.string())) {
                    const auto& h = ff::download_file(*database, ff::UserProperties{
                        .username = request.session.find("username") != request.session.end() ? request.session.at("username") : "",
                        .ip_address = request.ip_address,
                        .user_agent = request.user_agent,
                    }, file_path.string());

                    limhamn::http::server::response response{};

                    response.body = open_file(h.path);
                    response.http_status = 200;
                    response.content_type = limhamn::http::utils::get_appropriate_content_type(h.name);

                    if (settings.preview_files) {
                        response.headers.push_back({"Content-Disposition", "inline; filename=\"" + h.name + "\""});
                    } else {
                        response.headers.push_back({"Content-Disposition", "attachment; filename=\"" + h.name + "\""});
                    }

                    return response;
                }
            } else if (file.find("/view/") != std::string::npos) {
                std::filesystem::path file_path = file;
                file_path = file_path.lexically_normal(); // normalize the path

                if (file_path.string().find("/view/") == 0) {
                    return handlers.at("/")(request, *database);
                }
            }

            // handle activation URLs
            if (file.find("/activate/") != std::string::npos && settings.enable_email_verification) {
                int64_t ct = scrypto::return_unix_timestamp();
                const auto& list = database->query("SELECT * FROM activation_urls WHERE url = ?;", file);
                for (const auto& it : list) {
                    try {
                        const auto json = get_json_from_table(*database, "users", "username", it.at("username"));
                        nlohmann::json user_json = nlohmann::json::parse(json);

                        user_json["activated"] = true;

                        set_json_in_table(*database, "users", "username", it.at("username"), user_json.dump());

                        database->exec("DELETE FROM activation_urls WHERE url = ?;", file);

                        // redirect to /
                        limhamn::http::server::response response{};
                        response.http_status = 302;
                        response.headers.push_back({"Location", "/"});
                        return response;
                    } catch (const std::exception& e) {
                        limhamn::http::server::response resp;
                        resp.content_type = "text/html";
                        resp.http_status = 500;
                        resp.body = "<p>500 Internal Server Error</p>";
                        return resp;
                    }
                }
            }

            limhamn::http::server::response response{};

            response.content_type = "text/html";
            response.http_status = 404;
            response.body = "<p>404 Not Found</p>";

            return response;
        });
    } catch (const std::exception& e) {
        ff::logger.write_to_log(limhamn::logger::type::error, "An error occurred: " + std::string{e.what()} + "\n");

        // a little bit ugly but whatever
        if (std::string(e.what()).find("Address already in use") != std::string::npos) {
            ff::fatal = true;
        }

        if (ff::fatal) {
            ff::logger.write_to_log(limhamn::logger::type::error, "The last error was too severe to recover, and the server will now halt.\n");
            std::exit(EXIT_FAILURE);
        }

        if (ff::settings.halt_on_error) {
            ff::logger.write_to_log(limhamn::logger::type::error, "Halting the server due to an error.\n");
            std::exit(EXIT_FAILURE);
        }

        start_server();
    }
}

std::string ff::get_temp_path() {
    std::string ret = settings.temp_directory + "/" + scrypto::generate_random_string(32);
    while (std::filesystem::exists(ret)) {
        ret = settings.temp_directory + "/" + scrypto::generate_random_string(32);
    }
    return ret;
}
