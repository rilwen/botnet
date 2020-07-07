#pragma once
#include <memory>
#include <unordered_set>
#include "enums_json.hpp"
#include "json_fwd.hpp"
#include "user_environment_type.hpp"

namespace simbo {

	class JumpMarkovProcess;
	template <class T> class MarkovProcess;
	template <class T> class RandomVariable;
	class Schedule;

	/// Konfiguracja służąca do generowania nowych użytkowników.
	struct UserConfig {
		/// Wszystkie środowiska w których użytkownik korzysta z komputerów.
		std::vector<UserEnvironmentType> environments;

		/// Wszystkie środowiska w których użytkownik może czytać pocztę prywatną. Pusty zbiór oznacza że wszystkie środowiska są dozwolone.
		std::unordered_set<UserEnvironmentType> personal_email_environments;

		/// Wszystkie środowiska w których użytkownik może czytać pocztę pracową. Pusty zbiór oznacza że wszystkie środowiska są dozwolone.
		std::unordered_set<UserEnvironmentType> work_email_environments;

		/// Proces opisujący przejścia pomiędzy środowiskami (indeksy muszą odpowiadać wektorowi "environments").
		std::shared_ptr<MarkovProcess<int>> environment_process;

		/// \see User.
		std::shared_ptr<const RandomVariable<double>> probability_email_activated;

		/// \see User.
		std::shared_ptr<const RandomVariable<double>> initial_email_opening_probability;

		/// \see User.
		std::shared_ptr<const RandomVariable<double>> email_opening_probability_multiplier_exponent;

		/// \see User.
		std::shared_ptr<const RandomVariable<double>> probability_instant_email_check;

		/// Proces modelujący kiedy użytkownik sprawdza osobiste konto email. Nie może być null.
		std::shared_ptr<JumpMarkovProcess> personal_email_check_process;

		/// Proces modelujący kiedy użytkownik sprawdza pracowe konto email. Może być null, wtedy uznajemy że użytkownik nie ma pracowego konta pocztowego.
		std::shared_ptr<JumpMarkovProcess> work_email_check_process;

		/// Liczba użytkowników o tej konfiguracji. Musi być dodatnia.
		int count;

		/// Czy używa swojego laptopa, czy desktopów.
		bool uses_laptop;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt UserConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję UserConfig#validate().
	void from_json(const json& j, const Schedule& schedule, UserConfig& config);
}
