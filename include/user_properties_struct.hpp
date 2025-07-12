#pragma once

#include <string>

namespace ff {
    struct UserProperties {
        std::string username{}; /* only filled in if cookie is valid */
        std::string ip_address{};
        std::string user_agent{};
    };
} // namespace ff
