#pragma once
#include <memory>
#include <boost/locale.hpp>
#include "interpolator1d.hpp"
#include "markov_process.hpp"

namespace simbo {

	/**
	\brief Proces którego historia jest znana zawczasu.

	\tparam State Typ reprezentujący stan procesu.
	*/
	template <class State> class PredeterminedProcess : public MarkovProcess<State> {
	public:
		typedef std::unique_ptr<const Interpolator1D<seconds_t, State>> values_interpolator_ptr;

		/**
		\brief Konstruktor
		\param history Interpolator opisujący historię procesu.
		\throws std::invalid_argument Jeżeli interpolator jest null. Jeżeli historia nie jest określona na niezerowym odcinku.
		*/
		PredeterminedProcess(values_interpolator_ptr&& history)
			: history_(std::move(history)) {
			if (!history_) {
				throw std::invalid_argument(boost::locale::translate("Null history"));
			}
			if (history_->get_lower_bound() == history_->get_upper_bound()) {
				throw std::invalid_argument(boost::locale::translate("Zero history"));
			}
		}

		seconds_t get_first_time() const override {
			return history_->get_lower_bound();
		}

		seconds_t get_last_time() const override {
			return history_->get_upper_bound();
		}

		State get_next_state(RNG&, const State& previous_state, seconds_t previous_time, seconds_t next_time) const override {
			check_times(previous_time, next_time);
			if (previous_state != (*history_)(previous_time)) {
				throw std::domain_error(boost::locale::translate("Previous state incompatible with history"));
			}
			return (*history_)(next_time);
		}

		/// Zwraca wartość z historii odpowiadającą podanemu czasowi.
		State instant_switch(RNG&, const State& /*other_process_state*/, seconds_t time) const override {
			check_time(time);
			return (*history_)(time);
		}
	private:
		values_interpolator_ptr history_;
	};
}
