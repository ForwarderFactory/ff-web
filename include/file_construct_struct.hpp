#pragma once

#include <string>

namespace ff {
    struct FileConstruct {
        std::string path{};
        std::string name{};
        std::string username{};
        std::string ip_address{};
        std::string user_agent{};
    };
} // namespace ff
