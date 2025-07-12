#pragma once

#include <string>
#include <settings.hpp>
#include <account_creation_status_enum.hpp>
#include <upload_status_enum.hpp>
#include <login_status_enum.hpp>
#include <profile_update_status_enum.hpp>
#include <user_type_enum.hpp>
#include <file_construct_struct.hpp>
#include <retrieved_file_struct.hpp>
#include <user_properties_struct.hpp>
#include <cache_manager.hpp>
#include <database.hpp>
#define LIMHAMN_LOGGER_IMPL
#include <limhamn/logger/logger.hpp>
#define LIMHAMN_HTTP_SERVER_IMPL
#define LIMHAMN_HTTP_UTILS_IMPL
#include <limhamn/http/http_server.hpp>

namespace ff {
    inline limhamn::logger::logger logger{};
    inline CacheManager cache_manager{};
    inline bool fatal{false};
    inline static const std::string virtual_stylesheet_path{"/css/index.css"};
    inline static const std::string virtual_font_path{"/fonts/font.ttf"};
    inline static const std::string virtual_favicon_path{"/img/favicon.svg"};
    inline static const std::string virtual_script_path{"/js/index.js"};
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
    bool generate_thumbnail(const std::string& input, const std::string& output);
    std::string get_temp_path();

    std::string get_default_profile();

    void update_to_latest(database& db);

    std::string upload_file(database& db, const ff::FileConstruct& c);
    RetrievedFile download_file(database& db, const ff::UserProperties& prop, const std::string& file_key);
    std::string get_path_from_file(database& db, const std::string& file_key);
    void create_patched_dol(const std::string& path, const std::string& output_path);

    bool is_file(database& db, const std::string& file_key);
} // namespace ff
