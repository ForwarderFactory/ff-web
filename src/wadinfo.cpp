#include <ff.hpp>

ff::WADInfo ff::get_info_from_wad(const std::string& wad_path) {
    const auto recv = [](const std::string&& cmd) -> std::string {
        std::array<char, 128> buffer{};
        std::string result{};
        std::unique_ptr<FILE, decltype(&pclose)> pipe{popen(cmd.c_str(), "r"), pclose};
        if (!pipe) {
            throw std::runtime_error{"popen() failed!"};
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return std::move(result);
    };

    const auto extract_value = [](const std::string& output, const std::string& name) -> std::string {
        std::stringstream ss{output};
        std::string line;
        const std::string prefix = name + ": ";
        while (std::getline(ss, line)) {
            if (line.rfind(prefix, 0) == 0) {
                return line.substr(prefix.size());
            }
        }
        return {};
    };

    if (!std::filesystem::is_regular_file(wad_path)) {
        throw std::runtime_error{"WAD file does not exist."};
    }

    std::string command = "Sharpii WAD -i " + wad_path;
    std::string output = recv(std::move(command));

    ff::WADInfo ret{};

    ret.title = extract_value(output, "Title");
    ret.title_id = extract_value(output, "Title ID");
    ret.full_title_id = extract_value(output, "Full Title ID");
    ret.ios = std::stoi(extract_value(output, "IOS"));
    ret.region = extract_value(output, "Region");
    ret.version = std::stoi(extract_value(output, "Version"));
    ret.blocks = std::stoi(extract_value(output, "Blocks"));

    if (ret.title_id.empty() || ret.full_title_id.empty()) {
        throw std::runtime_error{"Failed to extract title or full title ID from WAD file."};
    }

    ret.supports_vwii = (ret.ios != 61 && ret.title_id.at(0) != 'E' && ret.title_id.at(0) != 'F' && ret.title_id.at(0) != 'J' &&
                        ret.title_id.at(0) != 'L' && ret.title_id.at(0) != 'M' && ret.title_id.at(0) != 'N' &&
                        ret.title_id.at(0) != 'P' && ret.title_id.at(0) != 'Q' && ret.title_id.at(0) != 'X');

    return ret;
}