#pragma once
#include <memory>
#include "botnet.hpp"
#include "schedule.hpp"

namespace simbo {

	class RNG;


	/** \brief Kontekst dzielony pomiędzy aktorami symulacji.
	*/
	class Context {
	public:
		/// Domyślny konstruktor tworzący pusty kontekst.
		Context();

		/** Standardowy konstruktor.

		\param rng Wskaźnik do generatora liczb pseudolosowych.
		\param botnet Wskaźnik do administratora botnetu.
		\param schedule Harmonogram symulacji.

		\throw std::invalid_argument Jeżeli rng == nullptr albo botnet == nullptr.
		*/
		Context(std::shared_ptr<RNG> rng, std::unique_ptr<Botnet>&& botnet, Schedule&& schedule);

		/// Konstruktor przenoszący.
		/// \param other Inny kontekst.
		Context(Context&& other);

		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		/// Przenoszący operator przypisania.
		/// \param other Inny obiekt Context.
		/// \return Referencja do obiektu po przypisaniu.
		Context& operator=(Context&& other);

		/// Zwróć referencję do generatora liczb pseudolosowych.
		RNG& get_rng() {
			return *rng_;
		}

		/// Zwróć referencję do botnetu.
		Botnet& get_botnet() {
			return *botnet_;
		}

		/// Zwróć referencję do botnetu.
		const Botnet& get_botnet() const {
			return *botnet_;
		}

		/// Zwróć konfigurację botnetu.
		const BotnetConfig& get_botnet_config() const;

		/// Zwróć ilość czasu od początku symulacji.
		seconds_t get_time() const {
			return current_time_;
		}

		/// Zwróć aktualny punkt czasowy symulacji.
		DateTime get_datetime() const;

		/// Zwróć ilość czasu od początku symulacji, w następnym jej kroku.
		seconds_t get_next_time() const {
			return next_time_;
		}

		/// Zwróć indeks punktu czasowego symulacji.
		int get_time_point_index() const {
			return time_point_index_;
		}

		/// Zwróć liczbę punktów czasowych.
		int get_number_time_points() const {
			return schedule_.size();
		}

		/// Zwróć referencję do harmonogramu symulacji.
		const Schedule& get_schedule() const {
			return schedule_;
		}

		/// Sprawdź czy istnieje następny punkt czasowy.
		bool has_next_time() const {
			return get_time_point_index() + 1 < get_number_time_points();
		}

		/** \brief Przejdź do następnego punktu czasowego.
		Funkcja powinna być wołana wyłącznie z SimulatorController albo z testów jednostkowych.
		\throw std::logic_error Jeżeli zwiększenie spowodowałoby że #get_time_point_index() == #get_number_time_points().
		*/
		void make_time_step();

		/** \brief Sprawdź czy czas zawiera się w aktualnym kroku czasowym.
		
		Zwraca wartość true jeżeli a) nie jesteśmy w ostatnim punkcie czasowym i time jest w przedziale [#get_time(), #get_next_time()), albo b) jesteśmy w ostatnim punkcie czasowym i time == #get_time();

		\param time Czas.
		*/
		bool is_inside_current_time_step(seconds_t time) const;
	private:
		std::shared_ptr<RNG> rng_;

		std::unique_ptr<Botnet> botnet_;

		Schedule schedule_;

		/// Indeks punktu czasowego.
		int time_point_index_;

		/// Aktualny czas w sekundach.
		seconds_t current_time_;

		/// Następny czas w sekundach.
		seconds_t next_time_;

		seconds_t get_next_time(int time_point_index) const;

		void wipe();
	};
}
