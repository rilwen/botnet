#pragma once
#include <boost/locale.hpp>
#include "conditional_markov_process.hpp"
#include "markov_process.hpp"

namespace simbo {
	/// Domyślna implementatacja klasy ConditionalMarkovProcess.
	template <class State, class Parameter> class ConditionalMarkovProcessImpl: public ConditionalMarkovProcess<State, Parameter> {
	public:
		typedef std::shared_ptr<const MarkovProcess<State>> markov_process_ptr;

		/**
		\brief Konstruktor.
		\param first_time Początkowy czas procesu.
		\param last_time Końcowy czas procesu.
		\throws std::domain_error Jeżeli first_time >= last_time.
		*/
		ConditionalMarkovProcessImpl(seconds_t first_time, seconds_t last_time)
			: first_time_(first_time), last_time_(last_time) {
			if (first_time >= last_time) {
				throw std::domain_error(boost::locale::translate("First time should be strictly less than last time"));
			}
		}

		state_type get_next_state(RNG& rng, const state_type& previous_state, const Conditions& previous_conditions, const Conditions& next_conditions) const override {
			check_times(previous_conditions.time, next_conditions.time);
			const auto next_process = get_markov_process(next_conditions.parameter);
			assert(next_process);
			state_type start_state_for_next_process = previous_state;
			if (previous_conditions.parameter != next_conditions.parameter) {
				// Przeskocz na nowy proces.
				start_state_for_next_process = next_process->instant_switch(rng, previous_state, previous_conditions.time);
			}
			return next_process->get_next_state(rng, start_state_for_next_process, previous_conditions.time, next_conditions.time);
		}

		seconds_t get_first_time() const override {
			return first_time_;
		}

		seconds_t get_last_time() const override {
			return last_time_;
		}
	protected:
		/// Zwróc wskaźnik do procesu markova odpowiadającego danemu parametrowi.
		virtual markov_process_ptr get_markov_process(const Parameter& parameter) const = 0;
	private:
		/// Początek zakresu czasowego.
		seconds_t first_time_;

		/// Koniec zakresu czasowego.
		seconds_t last_time_;
	};
}
