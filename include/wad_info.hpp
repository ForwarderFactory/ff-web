#pragma once

namespace ff {
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
	void replace_dol_in_wad(const std::string& wad, const std::string& dol);
	void set_ios_in_wad(const std::string& wad, int ios);
	void set_title_id_in_wad(const std::string& wad, const std::string& title_id);
} // namespace ff
