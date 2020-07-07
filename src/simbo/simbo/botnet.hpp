#pragma once
#include <unordered_map>
#include <unordered_set>
#include "botnet_config.hpp"
#include "bot_state.hpp"
#include "date_time.hpp"
#include "sinkhole.hpp"

namespace simbo {

	class Antivirus;
	class Context;
	class Host;
	class RNG;
	class OperatingSystem;

	/**
	\brief Botnet.
	Klasa modeluje cały botnet. Zawiera również zbiór wskaźników do hostów należących do botnetu (botów).
	*/
	class Botnet {
	public:
		/** \brief Konstruktor
		\param config Konfiguracja botnetu.
		\param sinkhole Sinkhole zbierający informacje o botnecie.
		\throws std::domain_error Zobacz BotnetConfig#validate()
		\throws std::invalid_argument Zobacz BotnetConfig#validate()
		*/
		Botnet(const BotnetConfig& config, Sinkhole&& sinkhole);

		/// Konstruktor przenoszący.
		Botnet(Botnet&&) = default;

		Botnet(const Botnet&) = delete;
		Botnet& operator=(const Botnet&) = delete;

		/** \brief Host dołącza do botnetu.
		Rejestruje wiadomość od hosta. Funkcja wołana przez obiekty klasy Host.
		\param ctx Kontekst symulacji.
		\param host Wskaźnik do dodawanego hosta.
		\param time Czas dodania.
		\throw std::invalid_argument Jeżeli host jest null albo wyłączony. Jeżeli host nie ma dostępu do sieci. Jeżeli host nie jest zainfekowany. Jeżeli host należy już do botnetu. Jeżeli host ma inną rolę niż BotState::SETTING_UP.
		*/
		void add_host(const Context& ctx, Host* host, seconds_t time);

		/** \brief Usuń Host z botnetu. Funkcja wywoływana po usunięciu infekcji.
		\param host Wskaźnik do usuwanego hosta.
		\param time Czas usunięcia.
		\throw std::invalid_argument Jeżeli host jest null albo wyłączony. Jeżeli host jest zainfekowany. Jeżeli host nie należy do botnetu. Jeżeli host ma inną rolę niż BotState::NONE.
		*/
		void remove_host(Host* host, seconds_t time);

		/** \brief Host prosi o konfigurację.
		Rejestruje wiadomość od hosta.
		\param ctx Kontekst symulacji.
		\param host Wskaźnik do uaktualnianego hosta.
		\param time Czas aktualizacji.
		\return Nowy stan bota.
		\throw std::invalid_argument Jeżeli host jest null albo wyłączony. Jeżeli host nie ma dostępu do sieci. Jeżeli host nie jest zainfekowany. Jeżeli host nie należy do botnetu. Jeżeli host ma rolę BotState::NONE.
		*/
		BotState configure_host(Context& ctx, Host* host, seconds_t time);

		/** \brief Host wysyła zwykłe powiadomienie o tym, że działa.
		Rejestruje wiadomość od hosta.
		\param host Wskaźnik do hosta.
		\param time Czas powiadomienia.
		\throw Zob. #configure_host.
		*/
		void receive_ping(const Host* host, seconds_t time);

		/// Zwróć konfigurację botnetu.
		const BotnetConfig& get_config() const {
			return config_;
		}

		/// Rozmiar botnetu.
		int size() const {
			return static_cast<int>(hosts_.size());
		}

		/// Oblicz liczbę botów z daną edycją payloadu które są włączone, z dostępem do Internetu i w stanie BotState::PROPAGATING.
		/// \param payload_release_number Numer edycji głównej części kodu (payload).
		int calc_number_actively_propagating_bots(int payload_release_number) const;

		/// Sprawdź czy host należy do botnetu.
		bool contains_host(const Host* host) const;

		/// Zwróć referencję do sinkhole'a.
		const Sinkhole& get_sinkhole() const {
			return sinkhole_;
		}

		/// Zwróć referencję do sinkhole'a.
		Sinkhole& get_sinkhole() {
			return sinkhole_;
		}

		/** \brief Sprawdź czy host jest podatny na infekcję przez ten botnet.
		\param os Referencja do systemu operacyjnego hosta.
		\param av Wskaźnik do antywirusa hosta (null jeżeli nie ma).		
		*/
		bool is_infectable(const OperatingSystem& os, const Antivirus* av) const;

		/** \brief Sprawdź czy antywirus może wykryć i usunąć infekcję.
		\param av Referencja do antywirusa.
		\param payload_release_number Numer edycji głównej części kodu (payload).
		*/
		bool can_detect_and_remove(const Antivirus& av, int payload_release_number) const;

		const std::unordered_map<int, int>& get_host_counts_per_payload_release() const {
			return host_counts_per_payload_release_;
		}

		/** \brief Konwertuj mapę wag stanów bota do ciągłego wektora.
		\param map Mapa stan -> waga dla wartości BotState poza BotState::SETTING_UP i BotState::NONE.
		\return Wektor wag
		\throw std::invalid_argument Jeżeli map.empty().
		*/
		static std::vector<double> bot_state_weights_as_vector(const std::unordered_map<BotState, double>& map);
	private:
		BotnetConfig config_;

		std::unordered_set<Host*> hosts_;

		std::unordered_map<int, int> host_counts_per_payload_release_;

		Sinkhole sinkhole_;

		/// Zarejestruj wiadomość od bota.
		void register_bot_message(const Host* host, seconds_t time);
	};
}
