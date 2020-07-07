#include "log.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include <spdlog/sinks/stdout_sinks.h> // Ten plik musi być po log.hpp

namespace simbo {
	std::shared_ptr<spdlog::logger> get_logger() {
		auto logger = spdlog::get(get_default_logger_name());
		if (!logger) {
			logger = spdlog::stdout_logger_mt(get_default_logger_name());
		}
		return logger;
	}

	std::shared_ptr<spdlog::logger> get_tracker() {
		auto logger = spdlog::get(get_tracking_logger_name());
		if (!logger) {
			logger = spdlog::create<spdlog::sinks::null_sink_mt>(get_tracking_logger_name());
		}
		return logger;
	}

	std::shared_ptr<spdlog::logger> setup_file_logging(const std::string& filename, bool truncate) {
		try {
			auto logger = spdlog::basic_logger_mt(get_default_logger_name(), filename, truncate);
			return logger;
		} catch (const spdlog::spdlog_ex &e) {
			throw std::invalid_argument((boost::locale::format("File logger initialisation failed: {1}") % e.what()).str());
		}
	}

	std::shared_ptr<spdlog::logger> setup_file_tracking(const std::string& filename) {
		try {
			auto logger = spdlog::basic_logger_mt(get_tracking_logger_name(), filename, true);
			logger->set_pattern("%v");
			return logger;
		} catch (const spdlog::spdlog_ex &e) {
			throw std::invalid_argument((boost::locale::format("Tracking file logger initialisation failed: {1}") % e.what()).str());
		}
	}
}
