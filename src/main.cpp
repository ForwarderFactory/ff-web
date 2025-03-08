#include <iostream>
#include <string>
#define LIMHAMN_ARGUMENT_MANAGER_IMPL
#include <limhamn/argument_manager/argument_manager.hpp>
#include <ff.hpp>

int main(int argc, char** argv) {
    std::string config_file{};

    limhamn::argument_manager::argument_manager arg{argc, argv};

    arg.push_back("-h|--help|/h|/help|help", [](const limhamn::argument_manager::collection& c) {ff::print_help(); std::exit(EXIT_SUCCESS);});
    arg.push_back("-v|--version|/v|/version|version", [](const limhamn::argument_manager::collection& c) {ff::print_version(); std::exit(EXIT_SUCCESS);});
    arg.push_back("-c|--config-file|/c|/config-file", [&](limhamn::argument_manager::collection& c) {
        if (c.arguments.size() <= 1) {
            std::cerr << "The -c/--config flag requires a file to be specified.\n";
            std::exit(EXIT_FAILURE);
        }

        config_file = c.arguments.at(++c.index);
    });
    arg.push_back("-gc|--generate-config|/gc|/generate-config", [&](const limhamn::argument_manager::collection& c) {std::cout << ff::generate_default_config(); std::exit(EXIT_SUCCESS);});
    arg.execute([](const std::string& arg) {});

    ff::settings = ff::load_settings(config_file);

    arg = limhamn::argument_manager::argument_manager(argc, argv);

    arg.push_back("-p|--port|/p|/port", [&](limhamn::argument_manager::collection& c) {
        if (c.arguments.size() <= 1) {
            std::cerr << "The -p/--port flag requires a port number to be specified.\n";
            std::exit(EXIT_FAILURE);
        }

        ff::settings.port = std::stoi(c.arguments.at(++c.index));
    });
    arg.push_back("-he|--halt-on-error|/he|/halt-on-error", [&](const limhamn::argument_manager::collection& c) {ff::settings.halt_on_error = true;});
    arg.push_back("-nhe|--no-halt-on-error|/nhe|/no-halt-on-error", [&](const limhamn::argument_manager::collection& c) {ff::settings.halt_on_error = false;});
    arg.push_back("-c|--config-file|/c|/config-file", [&](limhamn::argument_manager::collection& c) {
        ++c.index;
    });

    arg.execute([](const std::string& arg) {
        std::cerr << "Unknown argument: " << arg << "\n";
        std::exit(EXIT_FAILURE);
    });

    limhamn::logger::logger_properties logger_properties{};

    logger_properties.access_log_file = ff::settings.access_file;
    logger_properties.warning_log_file = ff::settings.warning_file;
    logger_properties.error_log_file = ff::settings.error_file;
    logger_properties.notice_log_file = ff::settings.notice_file;
    logger_properties.output_to_std = ff::settings.output_to_std;
    logger_properties.log_access_to_file = ff::settings.log_access_to_file;
    logger_properties.log_warning_to_file = ff::settings.log_warning_to_file;
    logger_properties.log_error_to_file = ff::settings.log_error_to_file;
    logger_properties.log_notice_to_file = ff::settings.log_notice_to_file;

    ff::logger.override_properties(logger_properties);

    ff::logger.write_to_log(limhamn::logger::type::notice, "ff-web is now running on port " + std::to_string(ff::settings.port) + ".\n");
#ifdef FF_DEBUG
    ff::logger.write_to_log(limhamn::logger::type::notice, "Debug mode is enabled. Do not release this, or you will suffer the wrath of Satan himself, and the entire universe will collapse.\n");
#endif

    ff::prepare_wd();
    ff::start_server();

    return EXIT_SUCCESS;
}
