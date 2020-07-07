#include <fstream>
#include <iostream>
#include <cxxopts.hpp>
#include <boost/format.hpp>
#include "simbo/json.hpp"
#include "simbo/log.hpp"
#include "simbo/simulation_configurator.hpp"
#include "simbo/simulation_controller.hpp"
#include "simbo/version.hpp"

using namespace simbo;

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options("botnet_simulation", "SIMBO botnet simulator");
		options.add_options()
			("c,config", "Path to configuration file (JSON)", cxxopts::value<std::string>())
			("l,level", "Log level: error, warn, info, debug or trace", cxxopts::value<std::string>(std::string("info")))
			("log_filename", "Name for log file (default: log to stdout)", cxxopts::value<std::string>())
			("tracker_filename", "Name for tracking file (default: do not track)", cxxopts::value<std::string>())
			("h,help", "Help")
			("v,validate", "Validate config file and exit");
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

		std::string log_level("info");
		if (options_values.count("level")) {
			log_level = options_values["level"].as<std::string>();
		}
		if (log_level == "error") {
			get_logger()->set_level(spdlog::level::err);
		} else if (log_level == "warn") {
			get_logger()->set_level(spdlog::level::warn);
		} else if (log_level == "info") {
			get_logger()->set_level(spdlog::level::info);
		} else if (log_level == "debug") {
			get_logger()->set_level(spdlog::level::debug);
		} else if (log_level == "trace") {
			get_logger()->set_level(spdlog::level::trace);
		} else if (!log_level.empty()) {
			throw std::runtime_error((boost::format("Unknown log level: %s") % log_level).str());
		}

		get_logger()->info("Using Simbo {}.{}.{}", Version::major, Version::minor, Version::svn_revision);

		spdlog::flush_every(std::chrono::seconds(15));

		std::ifstream config_file(config_filename);
		json config;
		config_file >> config;
		config_file.close();		
		get_logger()->info("Parsed JSON config from file {}", config_filename);
		get_logger()->debug("JSON config:\n{}", config.dump(4));
		SimulationConfigurator sc;
		SimulationController simcon(sc.setup_simulation(config));
		if (options_values.count("validate")) {
			std::cout << "Configuration file " << config_filename << " validated successfully" << std::endl;
			return 0;
		}
		simcon.run();

		get_logger()->info("Finished");
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		get_logger()->error("Error: {}", e.what());
		return -1;
	}
}
