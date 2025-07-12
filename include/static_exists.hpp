#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <settings.hpp>

namespace ff {
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

	inline StaticExists static_exists{};
} // namespace ff
