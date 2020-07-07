#pragma once
#include "date_time.hpp"

namespace simbo {
	/// Śledzi "czas własny", mierzony wolniejszym zegarem niż czas "zegara ściennego".
	class OwnTimeCounter {
	public:
		explicit OwnTimeCounter(seconds_t init_wall_time);

		seconds_t get_own_time() const {
			return own_time_;
		}

		seconds_t get_last_update_wall_time() const {
			return last_update_wall_time_;
		}

		/** \brief Zresetuj czas własny.
		\param wall_time Czas "ścienny", od którego zaczynamy mierzyć od zera czas własny.
		\throw std::domain_error Jeżeli wall_time < #get_last_update_wall_time().
		*/
		void reset(const seconds_t wall_time);

		/** \brief Uaktualnij czas własny.
		\param wall_time Aktualny czas "ścienny".
		\param own_time_was_running Czy czas własny płynął w okresie od poprzedniego uaktualnienia.
		\throw std::domain_error Jeżeli wall_time < #get_last_update_wall_time().
		*/
		void update(const seconds_t wall_time, bool own_time_was_running);
	private:
		seconds_t own_time_;
		seconds_t last_update_wall_time_;
	};
}
