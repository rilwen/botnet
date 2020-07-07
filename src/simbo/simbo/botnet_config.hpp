#pragma once
#include "bot_state.hpp"
#include "botnet_dropper_config.hpp"
#include "botnet_payload_config.hpp"
#include "date_time.hpp"
#include "json_fwd.hpp"
#include "monotonic_piecewise_constant_interpolator1d.hpp"
#include "operating_system.hpp"
#include "piecewise_constant_interpolator1d.hpp"

namespace simbo {
	class ActionInterval;
	template <class> class SparseDiscreteRandomVariable;

	/// Konfiguracja botnetu.
	struct BotnetConfig {
		/** \brief Konstruktor.*/
		BotnetConfig();

		BotnetConfig(const BotnetConfig&) = default;

		/// Konfiguracja droppera.
		BotnetDropperConfig dropper;

		/// Konfiguracja głównego kodu, w funkcji numeru edycji.
		PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig> payload;
		
		/// Prawdopodobieństwo, że bot zostanie zrekonfigurowany zamiast kontynuowania w poprzednim stanie (nie dotyczy wyjścia ze stanu BotState::SETTING_UP), w funkcji czasu.
		PiecewiseConstantInterpolator1D<DateTime, double> probability_of_reconfiguration;

		/// Wagi przypisane stanom bota (BotState) po początkowej konfiguracji, w funkcji czasu. Wagi muszą być nieujemne.
		/// Mapa powinna zawierać wagi dla przynajmniej jednego stanu za wyjątkiem BotState::SETTING_UP i BotState::NONE.
		/// Wagi dla pominiętych stanów są ustawione na zero.
		PiecewiseConstantInterpolator1D<DateTime, std::shared_ptr<SparseDiscreteRandomVariable<BotState>>> bot_state_weights;

		/// Liczba zarażających emaili wysyłanych na 1 sekundę przez centrum dowodzenia botnetu, w funkcji czasu. Musi być nieujemna.
		PiecewiseConstantInterpolator1D<DateTime, double> infectious_email_sent_per_second_by_command_centre;

		/// Rozmiar listy adresów email posiadanej przez botnet. Musi być nieujemny.
		int email_address_list_size;		

		/// Harmonogram wypuszczania kolejnych edycji głównego kodu.
		MonotonicPiecewiseConstantInterpolator1D<DateTime, int> payload_release_schedule;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
		/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
		void validate() const;			
	};

	/// Odczytaj obiekt BotnetConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję BotnetConfig#validate().
	/// \param j Dane JSON.
	/// \param config Obiekt konfiguracji.
	/// \throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
	/// \throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
	void from_json(const json& j, BotnetConfig& config);
}

//namespace nlohmann {
//	template <>
//	struct adl_serializer<simbo::BotnetConfig> {
//		/// \brief Odczytaj obiekt BotnetConfig z formatu JSON.
//		/// Wymagane pola: patrz klasa BotnetConfig.
//		/// Po skonstruowaniu obiektu, from_json woła funkcję BotnetConfig#validate().
//		static simbo::BotnetConfig from_json(const json& j);
//	};
//}
