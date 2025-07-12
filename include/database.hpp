#pragma once

#define LIMHAMN_DATABASE_IMPL
#include <limhamn/database/database.hpp>

namespace ff {
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
                return SQLITE_HANDLE.query(query);
            } else {
                return POSTGRES_HANDLE.query(query);
            }
        }
        bool exec(const std::string& query) {
            if (!this->enabled_type) {
                return SQLITE_HANDLE.exec(query);
            } else {
                return POSTGRES_HANDLE.exec(query);
            }
        }
        template <typename... Args>
        std::vector<std::unordered_map<std::string, std::string>> query(const std::string& query, Args... args) {
            if (!this->enabled_type) {
                return SQLITE_HANDLE.query(query, args...);
            } else {
                return POSTGRES_HANDLE.query(query, args...);
            }
        }
        template <typename... Args>
        bool exec(const std::string& query, Args... args) {
            if (!this->enabled_type) {
                return SQLITE_HANDLE.exec(query, args...);
            } else {
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
} // namespace ff
