#include <iostream>
#include <fstream>
#include <cxxopts.hpp>
#include "simbo/json.hpp"
#include "simbo/log.hpp"
#include "simbo/simulation_configurator.hpp"
#include "simbo/simulation_controller.hpp"

using namespace simbo;

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options("configuration_demo", "Program which demonstrates how simbo configuration files work");
		options.add_options()
			("c,config", "Path to configuration file (JSON)", cxxopts::value<std::string>())
			("d,debug", "Set logging level to DEBUG")
			("log_filename", "Name for log file (default: log to stdout)", cxxopts::value<std::string>())
			("tracker_filename", "Name for tracking file (default: do not track)", cxxopts::value<std::string>())
			("h,help", "Help");
		options.parse_positional({ "config" });

		const auto options_values = options.parse(argc, argv);

		if (options_values.count("help")) {
			std::cout << options.help({ "" }) << std::endl;
			return 0;
		}

		if (!options_values.count("config")) {
			throw std::runtime_error("No configuration file provided (use -h for help with parameters)");
		}
		const std::string config_filename(options_values["config"].as<std::string>());

		if (options_values.count("log_filename")) {
			setup_file_logging(options_values["log_filename"].as<std::string>(), true);
		}
		if (options_values.count("tracker_filename")) {
			setup_file_tracking(options_values["tracker_filename"].as<std::string>());
		}

		if (options_values["debug"].as<bool>()) {
			get_logger()->set_level(spdlog::level::debug);
		}

		std::ifstream config_file(config_filename);
		json config;
		config_file >> config;
		config_file.close();
		get_logger()->info("Parsed JSON config from file {}", config_filename);
		get_logger()->debug("JSON config:\n{}", config.dump(4));
		SimulationConfigurator sc;
		SimulationController simcon(sc.setup_simulation(config));
		simcon.run();

		get_logger()->info("Finished");
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return -1;
	}
}
