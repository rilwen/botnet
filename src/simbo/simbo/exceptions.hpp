#pragma once
#include <stdexcept>
#include <boost/locale.hpp>

namespace simbo {
	/** \brief Wyjątek rzucany kiedy nie powiedzie się deserializacja.
	\param target_class Nazwa tworzonej klasy.
	\param input Dane w postaci zserializowanej (np. JSON).
	\param error Komunikat o błędzie.
	*/
	class DeserialisationError : public std::runtime_error {
	public:
		DeserialisationError(const std::string& target_class, const std::string& input, const std::exception& original_error);

		DeserialisationError(const std::string& target_class, const std::string& input, const std::string& original_error);

		const std::string& get_target_class() const {
			return target_class_;
		}

		const std::string& get_input() const {
			return input_;
		}

		const std::string& get_original_error() const {
			return original_error_;
		}
	private:
		std::string target_class_;
		std::string input_;
		std::string original_error_;
	};

	/** \brief Błąd konfiguracji.

	Wyjątek rzucany kiedy konfiguracja symulacji jest wadliwa.
	*/
	class ConfigurationError : public std::runtime_error {
	public:
		/**
		\param configuration_section Część konfiguracji w której jest problem.
		\param description Opis problemu.
		*/
		ConfigurationError(const std::string& configuration_section, const std::string& description);

		const std::string& get_configuration_section() const {
			return configuration_section_;
		}

		const std::string& get_description() const {
			return description_;
		}
	private:
		std::string configuration_section_;
		std::string description_;
	};
}
