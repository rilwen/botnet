#include <boost/locale.hpp>
#include "jump_markov_process.hpp"

namespace simbo {
	int JumpMarkovProcess::instant_switch(RNG&, const int& other_process_state, seconds_t time) const {
		check_time(time);
		return std::max(other_process_state, get_min_state());
	}

	void JumpMarkovProcess::generate_jumps(RNG& rng, const int& previous_state, seconds_t previous_time, seconds_t next_time, std::vector<seconds_t>& jump_times) const {
		check_times(previous_time, next_time);
		if (previous_state < get_min_state()) {
			throw std::domain_error(boost::locale::translate("Previous state below minimum value"));
		}
		seconds_t t = previous_time;
		auto next_state = previous_state;
		while (true) {			
			t = get_next_jump(rng, next_state, t, next_time);
			if (t < next_time) {
				++next_state;
				jump_times.push_back(t);
			} else {
				break;
			}
		}
	}

	int JumpMarkovProcess::get_next_state(RNG& rng, const int& previous_state, seconds_t previous_time, seconds_t next_time) const {
		check_times(previous_time, next_time);
		if (previous_state < get_min_state()) {
			throw std::domain_error(boost::locale::translate("Previous state below minimum value"));
		}
		seconds_t t = previous_time;
		auto next_state = previous_state;
		while (true) {			
			t = get_next_jump(rng, next_state, t, next_time);
			if (t < next_time) {
				++next_state;
			} else {
				break;
			}
		}
		return next_state;
	}
}
