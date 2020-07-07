#pragma once
#include <boost/locale.hpp>
#include "markov_process.hpp"

namespace simbo {
	/**
	\brief Proces o stałej wartości, określony na przedziale czasowym \f$ [-\infty, \infty] \f$.
	\tparam State Kopiowalna klasa albo typ przechowujący stan procesu.
	*/
	template <class State> class ConstantProcess: public MarkovProcess<State> {
	public:
		ConstantProcess(const State& value)
			: value_(value) {
		}

		seconds_t get_first_time() const override {
			return -infinite_time();
		}

		seconds_t get_last_time() const override {
			return infinite_time();
		}

		state_type get_next_state(RNG&, const state_type& previous_state, seconds_t /*previous_time*/, seconds_t /*next_time*/) const  override {
			if (previous_state != value_) {
				throw std::domain_error(boost::locale::translate("Previous state inconsistent with constant value"));
			}
			return value_;
		}

		state_type instant_switch(RNG&, const state_type& /*other_process_state*/, seconds_t /*time*/) const override {
			return value_;
		}
	private:
		State value_;
	};
}
