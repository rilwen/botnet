#pragma once
#include "exceptions.hpp"
#include "markov_process.hpp"
#include "tracked_value.hpp"

namespace simbo {

	class RNG;

	/// \brief Stan procesu Markowa.
	/// \tparam V Typ wartości
	template <class V> class MarkovProcessState : private TrackedValue<V> {
	public:
		/// Konstruktor.
		/// \param initial_value Początkowa wartość.
		/// \param initial_time Początkowy czas.
		/// \param by_process Czy początkowa wartość była wygenerowana przez proces (true) czy zewnętrzną interwencję (false).
		/// \throw std::domain_error Jeżeli początkowy czas jest nieokreślony.
		MarkovProcessState(const V& initial_value, seconds_t initial_time, bool by_process)
			: TrackedValue<V>(initial_value, initial_time),
			by_process_(by_process) {
			if (is_time_undefined(initial_time)) {
				throw std::domain_error(boost::locale::translate("Initial time is undefined"));
			}
		}

		/// Zwraca aktualną wartość.
		using TrackedValue<V>::get_value;

		/// Zwraca ostatni czas zmiany wartości.
		using TrackedValue<V>::get_last_change_time;

		/// Zwraca ostatni czas uaktualnienia wartości.
		using TrackedValue<V>::get_last_update_time;

		/** \brief Zaburz stan procesu zewnętrzną interwencją.
		\param value Nowa wartość.
		\param time Nowy czas.
		\throw std::domain_error Jeżeli time <= #get_last_update_time().
		*/
		void reset(const V& value, const seconds_t time) {			
			if (time > get_last_update_time()) {
				update(value, time);
			} else {
				throw std::domain_error(boost::locale::translate("Reset time is not past the last update time"));
			}
			by_process_ = false;
		}

		/** \brief Ewoluuj stan procesu zgodnie z jego dynamiką.
		Po wykonaniu funkcji, #get_last_update_time() zwraca wartość time_to.
		\param rng Generator liczb losowych.
		\param process Proces Markowa generujący nowe wartości stanu.
		\param time_to Docelowy czas ewolucji.
		\throw std::domain_error Jeżeli wartość stanu nie jest kompatybilna z procesem, albo jeżeli #get_last_update_time() >= time_to.
		*/
		void evolve(RNG& rng, const MarkovProcess<V>& process, seconds_t time_to) {
			const V new_value = by_process_ ? process.get_next_state(rng, get_value(), get_last_update_time(), time_to)
				: process.instant_switch(rng, get_value(), time_to);
			update(new_value, time_to);
			by_process_ = true;
		}

		/// Czy aktualna wartość była wygenerowana przez proces, czy przez zewnętrzną interwencję?
		bool is_by_process() const {
			return by_process_;
		}
	private:
		/// Czy wartość była wygenerowana przez proces, czy przez zaburzenie. 
		bool by_process_;
	};
}
