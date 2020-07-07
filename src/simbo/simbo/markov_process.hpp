#pragma once
#include <boost/locale.hpp>
#include "date_time.hpp"

namespace simbo {
	class RNG;

	/**
	\brief Abstrakcyjna klasa reprezentująca proces Markowa (bez pamięci).
	\tparam State Kopiowalna klasa albo typ przechowujący stan procesu.
	*/
	template <class State> class MarkovProcess {
	public:
		/// Typ stanu procesu.
		typedef State state_type;

		/// Wirtualny destruktor.
		virtual ~MarkovProcess() {

		}

		/// Zwróć pierwszy czas dla którego określony jest proces.
		virtual seconds_t get_first_time() const = 0;

		/// Zwróć ostatni czas dla którego określony jest proces. Gwarantowane #get_last_time() > #get_first_time().
		virtual seconds_t get_last_time() const = 0;

		/**
		\brief Zwróć stan procesu (losowy lub deterministyczny) w następnej chwili czasu.
		\param rng Referencja do generatora liczb losowych.
		\param previous_state Poprzedni stan procesu.
		\param previous_time Poprzedni czas (w sekundach).
		\param next_time Następny czas (w sekundach).
		\returns Stan w następnej chwili czasu.
		\throws std::domain_error Jeżeli next_time <= previous_time, previous_time < #get_first_time() albo next_time > #get_last_time(). Również jeżeli wartość previous_state jest niekompatybilna z naturą procesu w czasie previous_time (np. wartość ujemna dla geometrycznego ruchu Browna).
		*/
		virtual state_type get_next_state(RNG& rng, const state_type& previous_state, seconds_t previous_time, seconds_t next_time) const = 0;

		/**
		\brief Przełącz się natychmiastowo na ten proces z innego.
		\param rng Referencja do generatora liczb losowych.
		\param other_process_state Stan innego procesu (wyjściowy).
		\param time Czas przejścia.
		\returns Stan tego procesu po przejściu.
		\throws std::domain_error Jeżeli time < #get_first_time() albo time > #get_last_time(). Jeżeli wartość stanu innego procesu jest taka, że przełączenie jest niemożliwe.
		*/
		virtual state_type instant_switch(RNG& rng, const state_type& other_process_state, seconds_t time) const = 0;
	protected:
		void check_times(const seconds_t previous_time, const seconds_t next_time) const {
			if (next_time <= previous_time) {
				throw std::domain_error(boost::locale::translate("Next time is not larger than previous time"));
			}
			if (previous_time < get_first_time() || next_time > get_last_time()) {
				throw std::domain_error(boost::locale::translate("Previous and/or next time outside time domain"));
			}
		}

		void check_time(const seconds_t time) const {
			if (time < get_first_time() || time > get_last_time()) {
				throw std::domain_error(boost::locale::translate("Time outside time domain"));
			}
		}
	};
}
