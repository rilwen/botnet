#pragma once
#include <string>
#include "exceptions.hpp"
#include <spdlog/spdlog.h>

namespace simbo {
	/// Zwróć nazwę domyślnego loggera.
	inline constexpr const char* get_default_logger_name() {
		return "default";
	}

	/// Zwróć nazwę loggera służącego do śledzenia stanu symulacji.
	inline constexpr const char* get_tracking_logger_name() {
		return "tracker";
	}

	/// Zwróć wskaźnik do domyślnego loggera ("default"). Jeżeli żadnego loggera nie skonfigurowano, stwórz loggera logującego do konsoli i zwróć wskaźnik do niego.
	std::shared_ptr<spdlog::logger> get_logger();

	/// Zwróć wskaźnik do loggera śledzącego stan symulacji ("tracker"). Jeżeli żadnego loggera nie skonfigurowano, stwórz loggera kierującego dane do /dev/null.
	std::shared_ptr<spdlog::logger> get_tracker();

	/// Skonfiguruj domyślnego loggera aby logował do pliku o podanej nazwie.
	/// \param filename Nazwa pliku (pełna ścieżka).
	/// \param truncate Czy usuwać poprzednie dane z pliku o nazwie filename (jeżeli istnieje).
	/// \throw std::invalid_argument Jeżeli filename jest pusty.
	/// \return Wskaźnik do plikowego loggera.
	std::shared_ptr<spdlog::logger> setup_file_logging(const std::string& filename, bool truncate);

	/// Skonfiguruj śledzącego loggera ("tracker") aby logował do pliku o podanej nazwie.
	/// Jeżeli plik już istnieje, zostanie nadpisany.
	/// \param filename Nazwa pliku (pełna ścieżka).
	/// \throw std::invalid_argument Jeżeli filename jest pusty.
	/// \return Wskaźnik do plikowego loggera.
	std::shared_ptr<spdlog::logger> setup_file_tracking(const std::string& filename);

	/** \brief Zwróć wskaźnik do loggera o danej nazwie.
	\param name Nazwa loggera.
	\return Wskaźnik albo null jeżeli takiego loggera nie ma. 
	*/
	inline std::shared_ptr<spdlog::logger> get_logger(const std::string& name) {
		auto logger = spdlog::get(name);
		if (!logger) {
			throw std::invalid_argument((boost::locale::format("No logger with this name: {1}") % name).str());
		}
		return logger;
	}
}
