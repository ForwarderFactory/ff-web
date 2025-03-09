#include <filesystem>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>
#include <ff.hpp>

ff::Settings ff::load_settings(const std::string& _config_file) {
#ifndef FF_DEBUG
    std::string config_file{"/etc/ff/config.yaml"};
#else
    std::string config_file{"./config.yaml"};
#endif

    if (!_config_file.empty()) {
        config_file = _config_file;
    } else {
        const char* env = std::getenv("FF_CONFIG_FILE");

        if (env != nullptr) {
            config_file = env;
        }
    }

    if (!std::filesystem::exists(config_file)) {
        std::cerr << "Warning: The configuration file does not exist. Using default settings.\n";
        return {};
    }

#ifdef FF_DEBUG
    std::cout << "Loading configuration file: " << config_file << "\n";
#endif

    ff::Settings settings{};

    try
    {
        YAML::Node config = YAML::LoadFile(config_file);

        if (config["logger"]["output_to_std"]) settings.output_to_std = config["logger"]["output_to_std"].as<bool>();
        if (config["logger"]["halt_on_error"]) settings.halt_on_error = config["logger"]["halt_on_error"].as<bool>();
        if (config["logger"]["log_access_to_file"]) settings.log_access_to_file = config["logger"]["log_access_to_file"].as<bool>();
        if (config["logger"]["log_warning_to_file"]) settings.log_warning_to_file = config["logger"]["log_warning_to_file"].as<bool>();
        if (config["logger"]["log_error_to_file"]) settings.log_error_to_file = config["logger"]["log_error_to_file"].as<bool>();
        if (config["logger"]["log_notice_to_file"]) settings.log_notice_to_file = config["logger"]["log_notice_to_file"].as<bool>();
        if (config["account"]["username_min_length"]) settings.username_min_length = config["account"]["username_min_length"].as<std::size_t>();
        if (config["account"]["username_max_length"]) settings.username_max_length = config["account"]["username_max_length"].as<std::size_t>();
        if (config["account"]["password_min_length"]) settings.password_min_length = config["account"]["password_min_length"].as<std::size_t>();
        if (config["account"]["password_max_length"]) settings.password_max_length = config["account"]["password_max_length"].as<std::size_t>();
        if (config["account"]["allowed_characters"]) settings.allowed_characters = config["account"]["allowed_characters"].as<std::string>();
        if (config["account"]["allow_all_characters"]) settings.allow_all_characters = config["account"]["allow_all_characters"].as<bool>();
        if (config["account"]["allow_public_registration"]) settings.public_registration = config["account"]["allow_public_registration"].as<bool>();
        if (config["account"]["default_user_type"]) settings.default_user_type = config["account"]["default_user_type"].as<int>();
        if (config["account"]["enable_email_verification"]) settings.enable_email_verification = config["account"]["enable_email_verification"].as<bool>();
        if (config["filesystem"]["session_directory"]) settings.session_directory = config["filesystem"]["session_directory"].as<std::string>();
        if (config["filesystem"]["data_directory"]) settings.data_directory = config["filesystem"]["data_directory"].as<std::string>();
        if (config["filesystem"]["temp_directory"]) settings.temp_directory = config["filesystem"]["temp_directory"].as<std::string>();
        if (config["filesystem"]["html_file"]) settings.html_file = config["filesystem"]["html_file"].as<std::string>();
        if (config["filesystem"]["css_file"]) settings.css_file = config["filesystem"]["css_file"].as<std::string>();
        if (config["filesystem"]["js_file"]) settings.script_file = config["filesystem"]["js_file"].as<std::string>();
        if (config["filesystem"]["favicon_file"]) settings.favicon_file = config["filesystem"]["favicon_file"].as<std::string>();
        if (config["filesystem"]["access_file"]) settings.access_file = config["filesystem"]["access_file"].as<std::string>();
        if (config["filesystem"]["warning_file"]) settings.warning_file = config["filesystem"]["warning_file"].as<std::string>();
        if (config["filesystem"]["error_file"]) settings.error_file = config["filesystem"]["error_file"].as<std::string>();
        if (config["filesystem"]["notice_file"]) settings.notice_file = config["filesystem"]["notice_file"].as<std::string>();
        if (config["filesystem"]["cache_static"]) settings.cache_static = config["filesystem"]["cache_static"].as<bool>();
        if (config["filesystem"]["cache_exists"]) settings.cache_exists = config["filesystem"]["cache_exists"].as<bool>();
        if (config["database"]["type"]) settings.enabled_database = config["database"]["type"].as<std::string>() == "postgresql";
        if (config["sqlite3"]["sqlite_database_file"]) settings.sqlite_database_file = config["sqlite3"]["sqlite_database_file"].as<std::string>();
        if (config["postgresql"]["database"]) settings.psql_database = config["postgresql"]["database"].as<std::string>();
        if (config["postgresql"]["username"]) settings.psql_username = config["postgresql"]["username"].as<std::string>();
        if (config["postgresql"]["password"]) settings.psql_password = config["postgresql"]["password"].as<std::string>();
        if (config["postgresql"]["host"]) settings.psql_host = config["postgresql"]["host"].as<std::string>();
        if (config["postgresql"]["port"]) settings.psql_port = config["postgresql"]["port"].as<int>();
        if (config["client"]["session_cookie_name"]) settings.session_cookie_name = config["client"]["session_cookie_name"].as<std::string>();
        if (config["site"]["url"]) settings.site_url = config["site"]["url"].as<std::string>();
        if (config["site"]["title"]) settings.title = config["site"]["title"].as<std::string>();
        if (config["site"]["description"]) settings.description = config["site"]["description"].as<std::string>();
        if (config["upload"]["max_request_size"]) settings.max_request_size = config["upload"]["max_request_size"].as<int64_t>();
        if (config["upload"]["max_file_size_hash"]) settings.max_file_size_hash = config["upload"]["max_file_size_hash"].as<int64_t>();
        if (config["download"]["preview_files"]) settings.preview_files = config["download"]["preview_files"].as<bool>();
        if (config["smtp"]["server"]) settings.smtp_server = config["smtp"]["server"].as<std::string>();
        if (config["smtp"]["port"]) settings.smtp_port = config["smtp"]["port"].as<int>();
        if (config["smtp"]["username"]) settings.email_username = config["smtp"]["username"].as<std::string>();
        if (config["smtp"]["password"]) settings.email_password = config["smtp"]["password"].as<std::string>();
        if (config["smtp"]["from"]) settings.email_from = config["smtp"]["from"].as<std::string>();
        if (config["http"]["port"]) settings.port = config["http"]["port"].as<int>();
        if (config["http"]["trust_x_forwarded_for"]) settings.trust_x_forwarded_for = config["http"]["trust_x_forwarded_for"].as<bool>();
        if (config["http"]["max_requests_per_ip_per_minute"]) settings.rate_limit = config["http"]["max_requests_per_ip_per_minute"].as<int>();
        if (config["http"]["whitelisted_ips"]) {
            for (const auto& ip : config["http"]["whitelisted_ips"]) {
                settings.whitelisted_ips.emplace_back(ip.as<std::string>());
            }
        }
        if (config["http"]["blacklisted_ips"]) {
            for (const auto& ip : config["http"]["blacklisted_ips"]) {
                settings.blacklisted_ips.emplace_back(ip.as<std::string>());
            }
        }
        if (config["paths"]) {
            for (const auto& n : config["paths"]) {
                auto first = n.first.as<std::string>();
                auto second = n.second.as<std::string>();
                if (std::filesystem::is_regular_file(second)) {
                    settings.custom_paths.emplace_back(n.first.as<std::string>(), n.second.as<std::string>());
                } else {
                    // allow * to match all files in a directory
                    if (second.back() == '*') {
                        // /thing/thing/* -> /thing/thing/it
                        // /thing/thing/* -> /thing/thing/it
                        for (const auto& entry : std::filesystem::directory_iterator(second.substr(0, second.size() - 1))) {
                            settings.custom_paths.emplace_back(first + entry.path().filename().string(), entry.path().string());
                        }
                    } else {
                        ff::logger.write_to_log(limhamn::logger::type::warning, "The file " + n.first.as<std::string>() + " does not exist. Skipping.\n");
                    }
                }
            }
        }
    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading the configuration file: " << e.what() << "\n";
        std::exit(EXIT_FAILURE);
    }

    return settings;
}

std::string ff::generate_default_config() {
    std::stringstream ss;

    ss << "# ff-web Configuration File\n";
    ss << "#\n";
    ss << "# This is the configuration file for ff-web version " << FF_VERSION << ". You can change the settings to your liking.\n";
    ss << "# It should be placed in /etc/ff/, named config.yaml. If you want to use a different file, you can set the FF_CONFIG_FILE environment variable or pass --config-file.\n";
    ss << "#\n";
    ss << "# Logger options:\n";
    ss << "#   output_to_std: Whether to output log messages to the standard output.\n";
    ss << "#   halt_on_error: Whether to halt the server on an error.\n";
    ss << "#   log_access_to_file: Whether to log access messages to a file.\n";
    ss << "#   log_warning_to_file: Whether to log warning messages to a file.\n";
    ss << "#   log_error_to_file: Whether to log error messages to a file.\n";
    ss << "#   log_notice_to_file: Whether to log notice messages to a file.\n";
    ss << "logger:\n";
    ss << "  output_to_std: " << (ff::settings.output_to_std ? "true" : "false") << "\n";
    ss << "  halt_on_error: " << (ff::settings.halt_on_error ? "true" : "false") << "\n";
    ss << "  log_access_to_file: " << (ff::settings.log_access_to_file ? "true" : "false") << "\n";
    ss << "  log_warning_to_file: " << (ff::settings.log_warning_to_file ? "true" : "false") << "\n";
    ss << "  log_error_to_file: " << (ff::settings.log_error_to_file ? "true" : "false") << "\n";
    ss << "  log_notice_to_file: " << (ff::settings.log_notice_to_file ? "true" : "false") << "\n";
    ss << "\n";
    ss << "# Account options:\n";
    ss << "#   username_min_length: The minimum length of a username.\n";
    ss << "#   username_max_length: The maximum length of a username.\n";
    ss << "#   password_min_length: The minimum length of a password.\n";
    ss << "#   password_max_length: The maximum length of a password.\n";
    ss << "#   allowed_characters: The characters allowed in a username or password.\n";
    ss << "#   allow_all_characters: Whether to allow all characters in a username or password.\n";
    ss << "#   allow_public_registration: Whether to allow public registration.\n";
    ss << "#   default_user_type: The default user type. (0 = User, 1 = Administrator)\n";
    ss << "#   enable_email_verification: Whether to enable email verification. Requires a valid, set up SMTP server.\n";
    ss << "account:\n";
    ss << "  username_min_length: " << ff::settings.username_min_length << "\n";
    ss << "  username_max_length: " << ff::settings.username_max_length << "\n";
    ss << "  password_min_length: " << ff::settings.password_min_length << "\n";
    ss << "  password_max_length: " << ff::settings.password_max_length << "\n";
    ss << "  allowed_characters: \"" << ff::settings.allowed_characters << "\"\n";
    ss << "  allow_all_characters: " << (ff::settings.allow_all_characters ? "true" : "false") << "\n";
    ss << "  allow_public_registration: " << (ff::settings.public_registration ? "true" : "false") << "\n";
    ss << "  default_user_type: " << ff::settings.default_user_type << "\n";
    ss << "  enable_email_verification: " << (ff::settings.enable_email_verification ? "true" : "false") << "\n";
    ss << "\n";
    ss << "# SMTP options:\n";
    ss << "#   server: The SMTP server.\n";
    ss << "#   port: The SMTP port.\n";
    ss << "#   username: The email username.\n";
    ss << "#   password: The email password.\n";
    ss << "#   from: The email from address.\n";
    ss << "smtp:\n";
    ss << "  server: \"" << ff::settings.smtp_server << "\"\n";
    ss << "  port: " << ff::settings.smtp_port << "\n";
    ss << "  username: \"" << ff::settings.email_username << "\"\n";
    ss << "  password: \"" << ff::settings.email_password << "\"\n";
    ss << "  from: \"" << ff::settings.email_from << "\"\n";
    ss << "\n";
    ss << "# HTTP options:\n";
    ss << "#   port: The port to run the web server on.\n";
    ss << "#   trust_x_forwarded_for: Whether to trust the X-Forwarded-For header. ONLY ENABLE IF YOU'RE USING A REVERSE PROXY THAT YOU TRUST!\n";
    ss << "#   max_requests_per_ip_per_minute: The maximum number of requests per IP per minute.\n";
    ss << "#   whitelisted_ips: A list of whitelisted IPs.\n";
    ss << "#   blacklisted_ips: A list of blacklisted IPs.\n";
    ss << "http:\n";
    ss << "  port: " << ff::settings.port << "\n";
    ss << "  trust_x_forwarded_for: " << (ff::settings.trust_x_forwarded_for ? "true" : "false") << "\n";
    ss << "  max_requests_per_ip_per_minute: " << ff::settings.rate_limit << "\n";
    ss << "  whitelisted_ips:\n";
    for (const auto& ip : ff::settings.whitelisted_ips) {
        ss << "    - " << ip << "\n";
    }
    ss << "  blacklisted_ips:\n";
    for (const auto& ip : ff::settings.blacklisted_ips) {
        ss << "    - " << ip << "\n";
    }
    ss << "\n";
    ss << "# Filesystem options:\n";
    ss << "#   session_directory: The directory where session files are stored.\n";
    ss << "#   data_directory: The directory where data files are stored.\n";
    ss << "#   temp_directory: The directory where temporary files are stored.\n";
    ss << "#   html_file: The path to the HTML file.\n";
    ss << "#   css_file: The path to the CSS file.\n";
    ss << "#   js_file: The path to the JS file.\n";
    ss << "#   favicon_file: The path to the favicon file.\n";
    ss << "#   access_file: The path to the access log file.\n";
    ss << "#   warning_file: The path to the warning log file.\n";
    ss << "#   error_file: The path to the error log file.\n";
    ss << "#   notice_file: The path to the notice log file.\n";
    ss << "#   cache_static: Whether to cache static files. (Experimental)\n";
    ss << "#   cache_exists: Whether to cache the existence of files. (Experimental)\n";
    ss << "filesystem:\n";
    ss << "  session_directory: \"" << ff::settings.session_directory << "\"\n";
    ss << "  data_directory: \"" << ff::settings.data_directory << "\"\n";
    ss << "  temp_directory: \"" << ff::settings.temp_directory << "\"\n";
    ss << "  html_file: \"" << ff::settings.html_file << "\"\n";
    ss << "  css_file: \"" << ff::settings.css_file << "\"\n";
    ss << "  js_file: \"" << ff::settings.script_file << "\"\n";
    ss << "  favicon_file: \"" << ff::settings.favicon_file << "\"\n";
    ss << "  access_file: \"" << ff::settings.access_file << "\"\n";
    ss << "  warning_file: \"" << ff::settings.warning_file << "\"\n";
    ss << "  error_file: \"" << ff::settings.error_file << "\"\n";
    ss << "  notice_file: \"" << ff::settings.notice_file << "\"\n";
    ss << "  cache_static: " << (ff::settings.cache_static ? "true" : "false") << "\n";
    ss << "  cache_exists: " << (ff::settings.cache_exists ? "true" : "false") << "\n";
    ss << "\n";
    ss << "# Database options:\n";
    ss << "#   type: The type of database to use. (sqlite3, postgresql)\n";
    ss << "database:\n";
    ss << "  type: \"" << (ff::settings.enabled_database ? "postgresql" : "sqlite3") << "\"\n";
    ss << "\n";
    ss << "# SQLite3 options:\n";
    ss << "#   sqlite_database_file: The path to the SQLite3 database file.\n";
    ss << "sqlite3:\n";
    ss << "  sqlite_database_file: \"" << ff::settings.sqlite_database_file << "\"\n";
    ss << "\n";
    ss << "# PostgreSQL options:\n";
    ss << "#   database: The PostgreSQL database.\n";
    ss << "#   username: The PostgreSQL username.\n";
    ss << "#   password: The PostgreSQL password.\n";
    ss << "#   host: The PostgreSQL host.\n";
    ss << "#   port: The PostgreSQL port.\n";
    ss << "postgresql:\n";
    ss << "  database: \"" << ff::settings.psql_database << "\"\n";
    ss << "  username: \"" << ff::settings.psql_username << "\"\n";
    ss << "  password: \"" << ff::settings.psql_password << "\"\n";
    ss << "  host: \"" << ff::settings.psql_host << "\"\n";
    ss << "  port: " << ff::settings.psql_port << "\n";
    ss << "\n";
    ss << "# Client options:\n";
    ss << "#   session_cookie_name: The name of the session cookie.\n";
    ss << "client:\n";
    ss << "  session_cookie_name: \"" << ff::settings.session_cookie_name << "\"\n";
    ss << "\n";
    ss << "# Site options:\n";
    ss << "#   url: The URL of the site (e.g. https://example.com).\n";
    ss << "#   title: The title of the site.\n";
    ss << "#   description: The description of the site.\n";
    ss << "site:\n";
    ss << "  url: \"" << ff::settings.site_url << "\"\n";
    ss << "  title: \"" << ff::settings.title << "\"\n";
    ss << "  description: \"" << ff::settings.description << "\"\n";
    ss << "\n";
    ss << "# Upload options:\n";
    ss << "#   max_request_size: The maximum request size in bytes. Any larger will be rejected by the server\n";
    ss << "#   max_file_size_hash: The maximum file size in bytes that can be hashed. Any larger will not be hashed.\n";
    ss << "upload:\n";
    ss << "  max_request_size: " << ff::settings.max_request_size << "\n";
    ss << "  max_file_size_hash: " << ff::settings.max_file_size_hash << "\n";
    ss << "\n";
    ss << "# Download options:\n";
    ss << "#   preview_files: Whether to preview files in the browser when downloading them.\n";
    ss << "download:\n";
    ss << "  preview_files: " << (ff::settings.preview_files ? "true" : "false") << "\n";
    ss << "\n";
    ss << "# Custom paths:\n";
    ss << "#   These are paths to files that are not in the default directories.\n";
    ss << "#   The first path is the virtual path, and the second path is the actual path.\n";
    ss << "paths:\n";
    for (const auto& p : ff::settings.custom_paths) {
        ss << "  \"" << p.first << "\": \"" << p.second << "\"\n";
    }

    return ss.str();
}
