#pragma once

#include <string>
#define LIMHAMN_LOGGER_IMPL
#define LIMHAMN_DATABASE_IMPL

#include <limhamn/database/database.hpp>
#include <limhamn/logger/logger.hpp>
#define LIMHAMN_HTTP_SERVER_IMPL
#define LIMHAMN_HTTP_UTILS_IMPL
#include <limhamn/http/http_server.hpp>

namespace ff {
    struct Settings {
#ifndef FF_DEBUG
        std::string access_file{"/var/log/ff/access.log"};
        std::string warning_file{"/var/log/ff/warning.log"};
        std::string error_file{"/var/log/ff/error.log"};
        std::string notice_file{"/var/log/ff/notice.log"};
        bool output_to_std{false};
        bool halt_on_error{false};
        std::string sqlite_database_file{"/var/db/ff/ff.db"};
        std::string session_directory{"/var/lib/ff/sessions"};
        std::string data_directory{"/var/lib/ff/data"};
        std::string temp_directory{"/var/tmp/ff"};
        std::string html_file{"/etc/ff/html/index.html"};
        std::string css_file{"/etc/ff/css/ff.css"};
        std::string script_file{"/etc/ff/js/ff.js"};
        std::string favicon_file{"/etc/ff/img/favicon.svg"};
        bool public_registration{true};
        std::vector<std::pair<std::string, std::string>> custom_paths{
            {"/img/pointer.png", "/etc/ff/img/pointer.png"},
            {"/img/pointer-moving.png", "/etc/ff/img/pointer-moving.png"},
            {"/img/grab.png", "/etc/ff/img/grab.png"},
            {"/img/grab-moving.png", "/etc/ff/img/grab-moving.png"},
            {"/img/background-logo-1.png", "/etc/ff/img/background-logo-1.png"},
            {"/img/discord.svg", "/etc/ff/img/discord.svg"},
            {"/img/star.svg", "/etc/ff/img/star.svg"},
            {"/img/pen.svg", "/etc/ff/img/pen.svg"},
            {"/img/logo.svg", "/etc/ff/img/logo.svg"},
            {"/img/announcements.svg", "/etc/ff/img/announcements.svg"},
            {"/fonts/font.woff2", "/etc/ff/fonts/font.woff2"},
            {"/audio/click.wav", "/etc/ff/audio/click.wav"},
        };
        int64_t max_request_size{250 * 1024 * 1024}; // 250mb
        std::string site_url{"https://forwarderfactory.com"};
        bool enable_email_verification{true};
#else
        std::string access_file{"./access.log"};
        std::string warning_file{"./warning.log"};
        std::string error_file{"./error.log"};
        std::string notice_file{"./notice.log"};
        std::string sqlite_database_file{"./ff-debug.db"};
        bool output_to_std{true};
        bool halt_on_error{false};
        std::string session_directory{"./sessions"};
        std::string data_directory{"./data"};
        std::string temp_directory{"./tmp"};
        std::string html_file{"./html/index.html"};
        std::string css_file{"./css/ff.css"};
        std::string script_file{"./js/ff.js"};
        std::string favicon_file{"./img/favicon.svg"};
        bool public_registration{true};
        std::vector<std::pair<std::string, std::string>> custom_paths{
            {"/img/pointer.png", "./img/pointer.png"},
            {"/img/pointer-moving.png", "./img/pointer-moving.png"},
            {"/img/grab.png", "./img/grab.png"},
            {"/img/grab-moving.png", "./img/grab-moving.png"},
            {"/img/background-logo-1.png", "./img/background-logo-1.png"},
            {"/img/discord.svg", "./img/discord.svg"},
            {"/img/star.svg", "./img/star.svg"},
            {"/img/pen.svg", "./img/pen.svg"},
            {"/img/logo.svg", "./img/logo.svg"},
            {"/img/announcements.svg", "./img/announcements.svg"},
            {"/fonts/font.woff2", "./fonts/font.woff2"},
            {"/audio/click.wav", "./audio/click.wav"},
        };
        int64_t max_request_size{1024 * 1024 * 1024};
        std::string site_url{"http://localhost:8080"};
        bool enable_email_verification{false};
#endif
        int port{8080};
        bool log_access_to_file{true};
        bool log_warning_to_file{true};
        bool log_error_to_file{true};
        bool log_notice_to_file{true};
        std::size_t password_min_length{8};
        std::size_t password_max_length{64};
        std::size_t username_min_length{3};
        std::size_t username_max_length{32};
        std::string allowed_characters{"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"};
        bool allow_all_characters{false};
        std::string session_cookie_name{"ff_session"};
        std::string title{"Forwarder Factory"};
        std::string description{"Forwarder Factory is a community dedicated to preserving and sharing Nintendo- and Wii-related content."};
        int default_user_type{0};
        bool preview_files{true};
        std::string email_username{};
        std::string email_password{};
        std::string email_from{};
        std::string smtp_server{};
        int smtp_port{465};
        std::string psql_username{"postgres"};
        std::string psql_password{"postgrespasswordhere"};
        std::string psql_database{"ff"};
        std::string psql_host{"localhost"};
        int psql_port{5432};
        bool enabled_database{false};
        bool trust_x_forwarded_for{false};
        int rate_limit{100};
        std::vector<std::string> blacklisted_ips{};
        std::vector<std::string> whitelisted_ips{"127.0.0.1"};
        int64_t max_file_size_hash{1024 * 1024 * 1024};
        bool cache_static{false};
        bool cache_exists{false};
        bool convert_images_to_webp{true};
        bool convert_videos_to_webm{false};
    };

    enum class AccountCreationStatus {
        Success,
        Failure,
        UsernameExists,
        UsernameTooShort,
        UsernameTooLong,
        PasswordTooShort,
        PasswordTooLong,
        InvalidUsername,
        InvalidPassword,
        InvalidEmail,
        EmailExists,
    };

    enum class UploadStatus {
        Success,
        Failure,
        InvalidCreds,
        NoFile,
        TooLarge,
    };

    enum class LoginStatus {
        Success,
        Failure,
        Inactive,
        InvalidUsername,
        InvalidPassword,
        Banned,
    };

    enum class ProfileUpdateStatus {
        Success,
        Failure,
        InvalidCreds,
        InvalidJson,
        InvalidIcon,
    };

    enum class UserType : int {
        Undefined = -1,
        User = 0,
        Administrator = 1,
    };

    struct FileConstruct {
        std::string path{};
        std::string name{};
        std::string username{};
        std::string ip_address{};
        std::string user_agent{};
    };

    struct RetrievedFile {
        std::string path{};
        std::string name{};
    };

    struct UserProperties {
        std::string username{}; /* only filled in if cookie is valid */
        std::string ip_address{};
        std::string user_agent{};
    };

    inline Settings settings{};

    class StaticExists {
        std::vector<std::pair<std::string, bool>> paths{};
    public:
        explicit StaticExists() = default;
        ~StaticExists() = default;
        [[nodiscard]] bool is_file(const std::string& path, const bool cache = true) {
            if (settings.cache_exists && cache) {
                for (const auto& it : this->paths) if (it.first == path) return it.second;
                paths.emplace_back(path, std::filesystem::exists(path));
                return paths.back().second;
            } else {
                return std::filesystem::exists(path);
            }
        }

    };

    class CacheManager {
        using FileContent = std::string;
        using FileName = std::string;
        std::vector<std::pair<FileName, FileContent>> cache{};
    public:
        explicit CacheManager() = default;
        ~CacheManager() = default;

        [[nodiscard]] FileContent open_file(const FileName& fp, const bool cache = true) { //NOLINT
            if (cache && settings.cache_static) {
                for (const auto& it : this->cache) if (it.first == fp) return it.second;
                if (!std::filesystem::exists(fp)) {
                    throw std::runtime_error{"File does not exist."};
                }
                std::ifstream file{fp};
                const std::string& data = {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
                this->cache.emplace_back(fp, data);
                return data;
            }

            std::ifstream file{fp};
            return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
        }
    };

    inline limhamn::logger::logger logger{};
    inline CacheManager cache_manager{};
    inline StaticExists static_exists{};
    inline bool fatal{false};

    class database {
#if FF_ENABLE_SQLITE
        limhamn::database::sqlite3_database sqlite{};
#define SQLITE_HANDLE this->sqlite
#endif
#if FF_ENABLE_POSTGRESQL
        limhamn::database::postgresql_database postgres{};
#define POSTGRES_HANDLE this->postgres
#endif
#ifndef SQLITE_HANDLE
#define SQLITE_HANDLE this->postgres
#endif
#ifndef POSTGRES_HANDLE
#define POSTGRES_HANDLE this->sqlite
#endif

        bool enabled_type = false; // false = sqlite, true = postgres
    public:
        explicit database(bool type) : enabled_type(type) {}
        std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query) {
            if (!this->enabled_type) {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "SQLite3 query: {{" + query + "}}\n");
#endif
                return SQLITE_HANDLE.query(query);
            } else {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "PostgreSQL query: {{" + query + "}}\n");
#endif
                return POSTGRES_HANDLE.query(query);
            }
        }
        bool exec(const std::string& query) {
            if (!this->enabled_type) {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "SQLite3 exec: {{" + query + "}}\n");
#endif
                return SQLITE_HANDLE.exec(query);
            } else {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "PostgreSQL exec: {{" + query + "}}\n");
#endif
                return POSTGRES_HANDLE.exec(query);
            }
        }
        template <typename... Args>
        std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query, Args... args) {
            if (!this->enabled_type) {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "SQLite3 query: {{" + query + "}}\n");
#endif
                return SQLITE_HANDLE.query(query, args...);
            } else {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "PostgreSQL query: {{" + query + "}}\n");
#endif
                return POSTGRES_HANDLE.query(query, args...);
            }
        }
        template <typename... Args>
        bool exec(const std::string& query, Args... args) {
            if (!this->enabled_type) {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "SQLite3 exec: {{" + query + "}}\n");
#endif
                return SQLITE_HANDLE.exec(query, args...);
            } else {
#if FF_DEBUG
                logger.write_to_log(limhamn::logger::type::notice, "PostgreSQL exec: {{" + query + "}}\n");
#endif
                return POSTGRES_HANDLE.exec(query, args...);
            }
        }
        [[nodiscard]] bool good() const {
            return this->enabled_type ? POSTGRES_HANDLE.good() : SQLITE_HANDLE.good();
        }
#if FF_ENABLE_SQLITE
        limhamn::database::sqlite3_database& get_sqlite() {
            return this->sqlite;
        }
#endif
#if FF_ENABLE_POSTGRESQL
        limhamn::database::postgresql_database& get_postgres() {
            return this->postgres;
        }
#endif
    };

    struct WADInfo {
        std::string title{};
        std::string title_id{};
        std::string full_title_id{};
        unsigned int ios{};
        std::string region{};
        int version{};
        int blocks{};
        bool supports_vwii{false};
    };

    WADInfo get_info_from_wad(const std::string& wad_path);

    inline static const std::string virtual_stylesheet_path = "/css/index.css";
    inline static const std::string virtual_font_path = "/fonts/font.ttf";
    inline static const std::string virtual_favicon_path = "/img/favicon.svg";
    inline static const std::string virtual_script_path = "/js/index.js";
    inline bool needs_setup{false};

    void start_server();
    std::string get_json_from_table(database& db, const std::string& table, const std::string& key, const std::string& value);
    bool set_json_in_table(database& db, const std::string& table, const std::string& key, const std::string& value, const std::string& json);
    void insert_into_user_table(database& database, const std::string& username, const std::string& password,
        const std::string& key, const std::string& email, int64_t created_at, int64_t updated_at, const std::string& ip_address,
        const std::string& user_agent, UserType user_type, const std::string& json);
    std::pair<LoginStatus, std::string> try_login(database& database, const std::string& username, const std::string& password,
        const std::string& ip_address, const std::string& user_agent, limhamn::http::server::response& response);
    AccountCreationStatus make_account(database& database, const std::string& username, const std::string& password, const std::string& email,
        const std::string& ip_address, const std::string& user_agent, UserType user_type);
    std::pair<UploadStatus, std::string> try_upload_forwarder(const limhamn::http::server::request& req, database& db);
    std::pair<UploadStatus, std::string> try_upload_file(const limhamn::http::server::request& req, database& db);
    ProfileUpdateStatus update_profile(const limhamn::http::server::request& req, database& db);

    std::string get_email_from_username(database& database, const std::string& username);
    std::string get_username_from_email(database& database, const std::string& email);
    void print_help(bool stream = false);
    void print_version(bool stream = false);
    void prepare_wd();
    Settings load_settings(const std::string& _config_file);
    std::string generate_default_config();
    void setup_database(database& database);
    std::string open_file(const std::string& file_path);
    bool username_is_stored(const limhamn::http::server::request& request);
    bool ensure_valid_creds(database& database, const std::string& username, const std::string& password);
    bool verify_key(database& database, const std::string& username, const std::string& key);
    bool user_is_verified(database& database, const std::string& username);
    bool ensure_admin_account_exists(database& database);
    int get_user_id(database& database, const std::string& username);
    UserType get_user_type(database& database, const std::string& username);
    bool validate_image(const std::string& path);
    bool validate_video(const std::string& path);
    bool convert_to_webm(const std::string& input, const std::string& output);
    bool convert_to_webp(const std::string& input, const std::string& output);
    std::string get_temp_path();

    std::string get_default_profile();

    void update_to_latest(database& db);

    std::string upload_file(database& db, const ff::FileConstruct& c);
    RetrievedFile download_file(database& db, const ff::UserProperties& prop, const std::string& file_key);
    bool is_file(database& db, const std::string& file_key);

    limhamn::http::server::response handle_root_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_try_setup_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_setup_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_virtual_favicon_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_virtual_stylesheet_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_virtual_script_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_try_register_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_try_login_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_try_upload_forwarder_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_try_upload_file_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_get_forwarders_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_get_files_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_set_approval_for_uploads_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_update_profile(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_get_profile(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_get_announcements(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_delete_announcements(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_create_announcement(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_rate_forwarder_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_rate_file_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_comment_forwarder_endpoint(const limhamn::http::server::request& request, database& db);
    limhamn::http::server::response handle_api_comment_file_endpoint(const limhamn::http::server::request& request, database& db);
} // namespace ff
