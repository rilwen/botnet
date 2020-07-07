#pragma once
#include "action_interval.hpp"
#include "json_fwd.hpp"

namespace simbo {
	/// Konfiguracja głównej części kodu infekującego hosta.
	struct BotnetPayloadConfig {
		/// Konstruktor.
		BotnetPayloadConfig();

		BotnetPayloadConfig(const BotnetPayloadConfig&) = default;		
		BotnetPayloadConfig(BotnetPayloadConfig&&) = default;
		BotnetPayloadConfig& operator=(const BotnetPayloadConfig&) = default;

		/// Długość okresu pomiędzy pingowaniem centrum dowodzenia przez bota.
		ActionInterval ping_interval;

		/// Czy bot ma pingować centrum dowodzenia po każdym uruchomieniu.
		bool ping_at_start;

		/// Długość okresu pomiędzy rekonfiguracją bota.
		ActionInterval reconfiguration_interval;

		/// Czy bot ma być rekonfigurowany po każdym uruchomieniu.
		bool reconfigure_at_start;

		/// Liczba zarażających emaili wysyłanych na 1 sekundę na 1 bota. Musi być nieujemna.
		double infectious_emails_sent_per_second_per_bot;

		/// Minimalny numer edycji antywirusa wykrywającej i usuwającej infekcję. Jeżeli mapa nie zawiera nazwy antywirusa, oznacza to że nie potrafi on nigdy wykryć ani usunąć infekcji.
		/// Nazwy nie mogą być puste.
		std::map<std::string, int> min_detecting_av_release;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
		/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt BotnetPayloadConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję BotnetPayloadConfig#validate().
	/// \param j Dane JSON.
	/// \param config Obiekt konfiguracji.
	/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
	/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
	void from_json(const json& j, BotnetPayloadConfig& config);
}

//namespace nlohmann {
//	template <>
//	struct adl_serializer<simbo::BotnetPayloadConfig> {
//		/// \brief Odczytaj obiekt BotnetConfig z formatu JSON.
//		/// Wymagane pola: patrz klasa BotnetPayloadConfig.
//		/// Po skonstruowaniu obiektu, from_json woła funkcję #validate().
//		static simbo::BotnetPayloadConfig from_json(const json& j);
//	};
//}
