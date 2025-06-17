#include <ff.hpp>
#include <scrypto.hpp>

ff::WADInfo ff::get_info_from_wad(const std::string& wad_path) {
	const auto recv = [](const std::string& cmd) -> std::string {
		std::array<char, 256> buffer{};
		std::string result;

		FILE* raw_pipe = popen(cmd.c_str(), "r");
		if (!raw_pipe) {
			throw std::runtime_error{"popen() failed"};
		}

		auto deleter = [](FILE* f) { if (f) pclose(f); };

		std::unique_ptr<FILE, decltype(deleter)> pipe(raw_pipe, deleter);

		while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
			result.append(buffer.data());
		}

		return result;
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

void ff::set_ios_in_wad(const std::string& wad, int ios) {
    auto temp_dir = std::filesystem::temp_directory_path();
    std::string input_file = (temp_dir / ("in-" + scrypto::generate_random_string(12))).string();
    std::string output_file_base = (temp_dir / ("out-" + scrypto::generate_random_string(12))).string();
    std::string output_file = output_file_base + ".wad";

    try {
        std::filesystem::copy(wad, input_file);
    } catch (const std::exception& e) {
        throw std::runtime_error{"Failed to copy WAD file: " + std::string(e.what())};
    }

    std::string command = "Sharpii WAD -e \"" + input_file + "\" \"" + output_file_base + "\" -ios " + std::to_string(ios);
#ifdef FF_DEBUG
    ff::logger.write_to_log(limhamn::logger::type::notice, "Command: " + command + "\n");
#endif
    if (std::system(command.c_str()) != 0) {
        throw std::runtime_error{"Failed to set IOS in WAD file. Command failed."};
    }

    try {
        std::filesystem::rename(output_file, wad);
    } catch (const std::exception& e) {
        throw std::runtime_error{"Rename failed: " + std::string(e.what())};
    }
}

void ff::set_title_id_in_wad(const std::string& wad, const std::string& title_id) {
    auto temp_dir = std::filesystem::temp_directory_path();
    std::string input_file = (temp_dir / ("in-" + scrypto::generate_random_string(12))).string();
    std::string output_file_base = (temp_dir / ("out-" + scrypto::generate_random_string(12))).string();
    std::string output_file = output_file_base + ".wad";

    try {
        std::filesystem::copy(wad, input_file);
    } catch (const std::exception& e) {
        throw std::runtime_error{"Failed to copy WAD file: " + std::string(e.what())};
    }

    std::string command = "Sharpii WAD -e \"" + input_file + "\" \"" + output_file_base + "\" -id " + title_id;
#ifdef FF_DEBUG
    ff::logger.write_to_log(limhamn::logger::type::notice, "Command: " + command + "\n");
#endif
    if (std::system(command.c_str()) != 0) {
        throw std::runtime_error{"Failed to set title ID in WAD file. Command failed."};
    }

    try {
        std::filesystem::rename(output_file, wad);
    } catch (const std::exception& e) {
        throw std::runtime_error{"Rename failed: " + std::string(e.what())};
    }
}

void ff::replace_dol_in_wad(const std::string& wad, const std::string& dol) {
    if (!std::filesystem::exists(dol)) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::notice, "DOL file does not exist: " + dol + "\n");
#endif
        throw std::runtime_error{"DOL file does not exist: " + dol};
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
        throw std::runtime_error{"Failed to copy files: " + std::string(e.what())};
    }

    if (!std::filesystem::exists(input_file) || !std::filesystem::exists(input_dol)) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::error, "Input file or DOL file does not exist after copy.\n");
#endif
        throw std::runtime_error{"Input file or DOL file does not exist after copy."};
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
        throw std::runtime_error{"Failed to replace DOL in WAD file. Command failed or output file does not exist."};
    }

    try {
        std::filesystem::rename(output_file, wad);
    } catch (const std::exception& e) {
#ifdef FF_DEBUG
        ff::logger.write_to_log(limhamn::logger::type::error, "Rename failed: " + std::string(e.what()) + "\n");
#endif
        throw std::runtime_error{"Rename failed: " + std::string(e.what())};
    }
}
