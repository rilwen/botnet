#pragma once
#include <boost/locale.hpp>
#include "date_time.hpp"

namespace simbo {

	class RNG;

	/**
	\brief Warunkowy proces Markowa.

	Proces X(t, Y(t)), który dla konkretnej trajektorii Y(t) jest procesem Markowa. Klasa modeluje przypadek w którym Y(t) jest stałe na przedziale \f$ [t_0, t_1) \f$. Wartość \f$ X(t_1) \f$ generujemy na podstawie wartości \f$ X(t_0), Y(t_0) \f$.

	\tparam State Typ wartości X(t).
	\tparam Parameter Typ wartości Y(t).
	*/
	template <class State, class Parameter> class ConditionalMarkovProcess {
	public:
		typedef State state_type;

		/// Para wartości: czas i parametr który obowiązywał bezpośrednio przed tym czasem.
		struct Conditions {
			/// Konstruktor domyślny.
			Conditions()
				: time(0) {
			}

			/// Prosty konstruktor.
			Conditions(seconds_t new_time, Parameter new_parameter)
				: time(new_time), parameter(new_parameter) {

			}

			/// Czas t (w sekundach).
			seconds_t time;

			/// Wartość parametru obowiązująca do chwili t.
			Parameter parameter;
		};

		/// Wirtualny destruktor.
		virtual ~ConditionalMarkovProcess() {
		}

		/// Zwróć pierwszy czas dla którego określony jest proces.
		virtual seconds_t get_first_time() const = 0;

		/// Zwróć ostatni czas dla którego określony jest proces. Gwarantowane #get_last_time() > #get_first_time().
		virtual seconds_t get_last_time() const = 0;

		/**
		\brief Zwróć stan procesu (losowy lub deterministyczny) w następnej chwili czasu.
		\param rng Generator liczb losowych.
		\param previous_state Poprzedni stan procesu.
		\param previous_conditions Poprzedni czas (t0) i parametr obowiązujący do t0.
		\param next_conditions Następny czas (t1) i parameter obowiązujący pomiędzy t0 a t1.
		\returns Stan w chwili czasu t1.
		\throws std::domain_error Jeżeli next_conditions.time <= previous_conditions.time, previous_conditions.time < #get_first_time() albo next_conditions.time > #get_last_time(). Również jeżeli wartość previous_state jest niekompatybilna z naturą procesu w czasie previous_conditions.time (np. wartość ujemna dla geometrycznego ruchu Browna).
		*/
		virtual state_type get_next_state(RNG& rng, const state_type& previous_state, const Conditions& previous_conditions, const Conditions& next_conditions) const = 0;
	protected:
		void check_times(const seconds_t previous_time, const seconds_t next_time) const {
			if (next_time <= previous_time) {
				throw std::domain_error(boost::locale::translate("Next time is not larger than previous time"));
			}
			if (previous_time < get_first_time() || next_time > get_last_time()) {
				throw std::domain_error(boost::locale::translate("Previous and/or next time outside time domain"));
			}
		}
	};
}
