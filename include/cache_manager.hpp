#pragma once

#include <filesystem>
#include <string>
#include <fstream>
#include <settings.hpp>

namespace ff {
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
} // namespace ff
