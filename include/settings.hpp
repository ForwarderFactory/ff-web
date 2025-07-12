#pragma once

#include <vector>
#include <string>

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
            {"/img/messages.svg", "/etc/ff/img/messages.svg"},
            {"/img/shovel.svg", "/etc/ff/img/shovel.svg"},
            {"/img/question-mark-block.svg", "/etc/ff/img/question-mark-block.svg"},
            {"/img/coin.svg", "/etc/ff/img/coin.svg"},
            {"/img/hammer.svg", "/etc/ff/img/hammer.svg"},
            {"/img/star.svg", "/etc/ff/img/star.svg"},
            {"/img/retro-star.svg", "/etc/ff/img/retro-star.svg"},
            {"/img/wave.svg", "/etc/ff/img/wave.svg"},
            {"/img/pen.svg", "/etc/ff/img/pen.svg"},
            {"/img/logo.svg", "/etc/ff/img/logo.svg"},
            {"/img/announcements.svg", "/etc/ff/img/announcements.svg"},
            {"/fonts/font.woff2", "/etc/ff/fonts/font.woff2"},
            {"/audio/click.wav", "/etc/ff/audio/click.wav"},
            {"/img/favicon.ico", "/etc/ff/img/favicon.ico"},
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
            {"/img/messages.svg", "./img/messages.svg"},
            {"/img/shovel.svg", "./img/shovel.svg"},
            {"/img/question-mark-block.svg", "./img/question-mark-block.svg"},
            {"/img/hammer.svg", "./img/hammer.svg"},
            {"/img/coin.svg", "./img/coin.svg"},
            {"/img/star.svg", "./img/star.svg"},
            {"/img/wave.svg", "./img/wave.svg"},
            {"/img/retro-star.svg", "./img/retro-star.svg"},
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
        bool topics_require_admin{false};
    };

    inline Settings settings{};
} // namespace ff
