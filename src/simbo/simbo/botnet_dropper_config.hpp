#pragma once
#include <map>
#include "date_time.hpp"
#include "json_fwd.hpp"
#include "operating_system.hpp"

namespace simbo {
	/// Konfiguracja "droppera" botnetu.
	struct BotnetDropperConfig {
		/// Domyślny konstruktor.
		BotnetDropperConfig();

		BotnetDropperConfig(const BotnetDropperConfig&) = default;
		/// Czas trwania konfiguracji po zarażeniu. Musi być dodatni.
		/// Konfiguracja zachodzi kiedy bot jest online (włączony i z dostępem do sieci).
		seconds_t setting_up_period;

		/// Maksymalna infekowana edycja dla danych typu i wersji systemu operacyjnego. Jeżeli mapa nie zawiera danej pary (typ, wersja),
		/// oznacza to że botnet nie infekuje tej pary.
		std::map<os_spec_t, int> max_infected_release;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
		/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt BotnetDropperConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję BotnetDropperConfig#validate().
	/// \param j Dane JSON.
	/// \param config Obiekt konfiguracji.
	/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
	/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
	void from_json(const json& j, BotnetDropperConfig& config);
}

//namespace nlohmann {
//	template <>
//	struct adl_serializer<simbo::BotnetDropperConfig> {
//		/// \brief Odczytaj obiekt BotnetDropperConfig z formatu JSON.
//		/// Wymagane pola: patrz klasa BotnetDropperConfig.
//		/// Po skonstruowaniu obiektu, from_json woła funkcję BotnetDropperConfig#validate().
//		static simbo::BotnetDropperConfig from_json(const json& j);
//	};
//}