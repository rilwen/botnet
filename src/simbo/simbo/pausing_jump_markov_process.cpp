#include "exceptions.hpp"
#include "json.hpp"
#include "jump_markov_process_json.hpp"
#include "pausing_jump_markov_process.hpp"
#include "simple_poisson_process.hpp"

namespace simbo {
	PausingJumpMarkovProcess::PausingJumpMarkovProcess(base_jump_process_ptr&& base_jump_process, seconds_t pause_length)
		: pause_length_(pause_length) {
		if (pause_length < 0) {
			throw std::domain_error(boost::locale::translate("Negative pause length"));
		}
		if (!base_jump_process) {
			throw std::invalid_argument(boost::locale::translate("Base jump process is null"));
		}
		base_jump_process_ = std::move(base_jump_process);
	}

	PausingJumpMarkovProcess::PausingJumpMarkovProcess(double lambda, seconds_t pause_length)
		: PausingJumpMarkovProcess(std::unique_ptr<const JumpMarkovProcess>(new SimplePoissonProcess(lambda)), pause_length) {}

	PausingJumpMarkovProcess::state_type PausingJumpMarkovProcess::get_next_state(RNG& rng, const state_type& previous_state, seconds_t previous_time, const seconds_t next_time) const {
		check_times(previous_time, next_time);
		seconds_t last_jump_time = previous_state.second;
		if (previous_time < last_jump_time) {
			throw std::invalid_argument(boost::locale::translate("Last jump time after previous process simulation time"));
		}
		seconds_t unpaused_start = std::max(previous_time, last_jump_time + pause_length_);		
		int value = previous_state.first;
		while (unpaused_start < next_time) {
			unpaused_start = base_jump_process_->get_next_jump(rng, value, unpaused_start, next_time);
			if (unpaused_start < next_time) {
				++value;
				last_jump_time = unpaused_start;
				unpaused_start += pause_length_;
			} else {
				assert(unpaused_start == next_time);
				break;
			}
		}
		return std::make_pair(value, last_jump_time);
	}

	PausingJumpMarkovProcess::state_type PausingJumpMarkovProcess::instant_switch(RNG&, const state_type& other_process_state, seconds_t time) const {
		// Zakładamy, że inny proces nie pauzuje.
		if (other_process_state.second > time) {
			throw std::domain_error(boost::locale::translate("Other process state comes from the future"));
		}
		return std::make_pair(other_process_state.first, time - pause_length_);
	}

	void from_json(const json& j, const Schedule& schedule, std::unique_ptr<PausingJumpMarkovProcess>& process) {
		validate_keys(j, "PausingJumpMarkovProcess", { "base_process", "pause_length" }, {});
		try {
			std::unique_ptr<JumpMarkovProcess> base_process;
			from_json(j["base_process"], schedule, base_process);
			const seconds_t pause_length = get_seconds_from_json(j["pause_length"]);
			process = std::make_unique<PausingJumpMarkovProcess>(std::move(base_process), pause_length);
		} catch (const std::exception& e) {
			throw DeserialisationError("PausingJumpMarkovProcess", j.dump(), e);
		}
	}
}
