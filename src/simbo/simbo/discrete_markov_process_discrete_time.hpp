#pragma once
#include <cassert>
#include <memory>
#include <vector>
#include <boost/locale.hpp>
#include "discrete_random_variable.hpp"
#include "interpolator1d.hpp"
#include "markov_process.hpp"

namespace simbo {

	/**
	\brief Dyskretny proces Markowa.

	DiscreteMarkovProcessDiscreteTime modeluje proces Markowa o dyskretnych wartościach z przedziału 0,..., D-1, ewoluujący w dyskretnym czasie.
	Wartość procesu zmienia się w chwilach T_i = T_0 + i * dt. Każdemu okresowi [T_i, T_{i+1}) przypisana jest macierz przejścia pi_i.
	Jeżeli w chwili t_start z przedziału \f$ [T_i, T_{i+1})\f$ proces mial wartość k, to prawdopodobieństwo że w chwili t_end z przedziału \f$ [T_{i+1}, T_{i+2}) \f$ proces
	będzie miał wartość l wynosi pi_i[l, k].

	\tparam State Typ reprezentujący stan procesu, konwertowalny na z/na wartości nieujemne typu int.
	*/
	template <class State> class DiscreteMarkovProcessDiscreteTime : public MarkovProcess<State> {
	public:
		// Deklaracje typów
		/// Typ macierzy przejścia.
		typedef std::vector<DiscreteRandomVariable> TransitionMatrix;
		/// Interpolator macierzy przejścia.
		typedef const Interpolator1D<int, const TransitionMatrix&> TransitionMatrixInterpolator;
		/// Wskaźnik do interpolatora macierzy przejścia.
		typedef std::unique_ptr<const TransitionMatrixInterpolator> interpolator_ptr;

		/**
		\brief Konstruktor.

		\param transition_matrix_interpolator Wskaźnik do interpolatora macierzy przejścia.
		\param zero_time Czas "zero", odpowiadający indeksowi 0 w interpolatorze macierzy przejścia.
		\param period Okres pomiędzy przejściami.
		\throws std::domain_error Jeżeli period <= 0. 
		\throws std::invalid_argument Jeżeli któryś z podanych wskaźników jest null. Jeżeli interpolator jest określony na przedziale czasu o zerowej długości.
		*/
		DiscreteMarkovProcessDiscreteTime(interpolator_ptr&& transition_matrix_interpolator, const seconds_t zero_time, const seconds_t period)
			: period_(period), zero_time_(zero_time) {
			if (period <= 0) {
				throw std::domain_error(boost::locale::translate("Period must be positive"));
			}
			if (transition_matrix_interpolator == nullptr) {
				throw std::invalid_argument(boost::locale::translate("Null implementation provided"));
			}
			if (transition_matrix_interpolator->get_lower_bound() == transition_matrix_interpolator->get_upper_bound()) {
				throw std::invalid_argument(boost::locale::translate("Process defined on zero range"));
			}
			first_time_ = zero_time + period * transition_matrix_interpolator->get_lower_bound();
			last_time_ = zero_time + period * transition_matrix_interpolator->get_upper_bound();
			transition_matrix_interpolator_ = std::move(transition_matrix_interpolator);
		}
		
		seconds_t get_first_time() const override {
			return first_time_;
		}

		seconds_t get_last_time() const override {
			return last_time_;
		}

		State get_next_state(RNG& rng, const State& previous_state, const seconds_t previous_time, const seconds_t next_time) const override {
			check_times(previous_time, next_time);
			auto period_idx = static_cast<int>(floor((previous_time - zero_time_) / period_));
			const auto next_period_idx = static_cast<int>(floor((next_time - zero_time_) / period_));
			if (next_period_idx == period_idx) {
				return previous_state;
			}
			auto state_idx = static_cast<int>(previous_state);
			while (period_idx < next_period_idx) {
				assert(state_idx >= 0);
				const auto& transition_matrix = (*transition_matrix_interpolator_)(period_idx);
				assert(state_idx < static_cast<int>(transition_matrix.size()));
				state_idx = transition_matrix[state_idx](rng);				
				++period_idx;
			}
			return static_cast<State>(state_idx);
		}

		/// Zwraca wartość other_process_state.
		State instant_switch(RNG&, const State& other_process_state, seconds_t time) const override {
			check_time(time);
			return other_process_state;
		}
	private:
		/// Interpolator macierzy przejścia.
		interpolator_ptr transition_matrix_interpolator_;

		/// Okres pomiędzy przejściami.
		seconds_t period_;

		/// Czas (w sekundach) kiedy pierwsza macierz przejścia zaczyna obowiązywać.
		seconds_t first_time_;

		/// Czas (w sekundach) kiedy ostatnia macierz przejścia przestaje obowiązywać.
		seconds_t last_time_;

		/// Czas "zero", odpowiadający indeksowi 0 w interpolatorze.
		seconds_t zero_time_;
	};

	
}
