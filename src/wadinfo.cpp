#include <ff.hpp>
#include <scrypto.hpp>

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

bool ff::replace_dol_in_wad(const std::string& wad, const std::string& dol) {
    if (!std::filesystem::exists(dol)) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::notice, "DOL file does not exist: " + dol + "\n");
#endif
        return false;
    }

    auto temp_dir = std::filesystem::temp_directory_path();
    std::string input_file = (temp_dir / ("in-" + scrypto::generate_random_string(12))).string();
    std::string input_dol  = (temp_dir / (scrypto::generate_random_string(12) + ".dol")).string();
    std::string output_file_base = (temp_dir / ("out-" + scrypto::generate_random_string(12))).string();
    std::string output_file = output_file_base + ".wad";

    try {
        std::filesystem::copy(wad, input_file);
        std::filesystem::copy(dol, input_dol);
    } catch (const std::exception& e) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::error, "Failed to copy files: " + std::string(e.what()) + "\n");
#endif
        return false;
    }

    if (!std::filesystem::exists(input_file) || !std::filesystem::exists(input_dol)) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::error, "Input file or DOL file does not exist after copy.\n");
#endif
        return false;
    }

    std::string command = "Sharpii WAD -e \"" + input_file + "\" \"" + output_file_base + "\" -dol \"" + input_dol + "\"";

#ifdef FF_DEBUG
    ff::logger.write_to_log(limhamn::logger::type::notice, "Command: " + command + "\n");
#endif

    int result_code = std::system(command.c_str());

#ifdef FF_DEBUG
    ff::logger.write_to_log(limhamn::logger::type::notice, "Exit code: " + std::to_string(result_code) + "\n");
#endif

#ifndef FF_DEBUG
    std::filesystem::remove(input_file);
    std::filesystem::remove(input_dol);
#endif

    if (result_code != 0 || !std::filesystem::exists(output_file)) {
        return false;
    }

    try {
        std::filesystem::rename(output_file, wad);
    } catch (const std::exception& e) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::error, "Rename failed: " + std::string(e.what()) + "\n");
#endif
        return false;
    }

    return std::filesystem::exists(wad);
}
