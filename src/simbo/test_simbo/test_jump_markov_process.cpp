#include <gtest/gtest.h>
#include "simbo/jump_markov_process.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class MockJumpMarkovProcess : public JumpMarkovProcess {
public:
	seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const override;

	int get_min_state() const override {
		return -2;
	}

	seconds_t get_first_time() const override {
		return 0.1;
	}

	seconds_t get_last_time() const override {
		return 1.1;
	}
};

seconds_t MockJumpMarkovProcess::get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const {
	check_times(previous_time, horizon);
	return std::min(previous_time + previous_count * 0.1, horizon);
}

TEST(JumpMarkovProcess, get_next_state_throws) {
	MockJumpMarkovProcess jmp;
	MockRNG rng;
	ASSERT_THROW(jmp.get_next_state(rng, 0, 0, 0.5), std::domain_error);
	ASSERT_THROW(jmp.get_next_state(rng, 0, 0, 1.5), std::domain_error);
	ASSERT_THROW(jmp.get_next_state(rng, 0, 1, 0.2), std::domain_error);
	ASSERT_THROW(jmp.get_next_state(rng, -3, 0.2, 0.3), std::domain_error);
}

TEST(JumpMarkovProcess, get_next_state) {
	MockJumpMarkovProcess jmp;
	MockRNG rng;
	ASSERT_EQ(1000, jmp.get_next_state(rng, 1000, 0.1, 1.1));
	ASSERT_EQ(2, jmp.get_next_state(rng, 1, 0.1, 0.4));
	ASSERT_EQ(3, jmp.get_next_state(rng, 1, 0.1, 0.400001));
	ASSERT_EQ(1, jmp.get_next_state(rng, 1, 0.1, 0.2));
	ASSERT_EQ(2, jmp.get_next_state(rng, 1, 0.1, 0.2000001));
}

TEST(JumpMarkovProcess, generate_jumps_throws) {
	MockJumpMarkovProcess jmp;
	std::vector<seconds_t> times;
	MockRNG rng;
	ASSERT_THROW(jmp.generate_jumps(rng, 0, 0, 0.5, times), std::domain_error);
	ASSERT_THROW(jmp.generate_jumps(rng, 0, 0, 1.5, times), std::domain_error);
	ASSERT_THROW(jmp.generate_jumps(rng, 0, 1, 0.2, times), std::domain_error);
	ASSERT_THROW(jmp.generate_jumps(rng, -3, 0.2, 0.3, times), std::domain_error);
}

TEST(JumpMarkovProcess, generate_jumps) {
	MockJumpMarkovProcess jmp;
	MockRNG rng;
	std::vector<seconds_t> times;
	times.push_back(-0.1);
	jmp.generate_jumps(rng, 1, 0.11, 1, times);
	const std::vector<seconds_t> expected_times({ 0.21, 0.41, 0.71 });
	ASSERT_EQ(expected_times.size() + 1, times.size());
	for (size_t idx = 0; idx < expected_times.size(); ++idx) {
		ASSERT_NEAR(expected_times[idx], times[idx + 1], 1e-14) << idx;
	}
}

TEST(JumpMarkovProcess, instant_switch_throws) {
	MockJumpMarkovProcess jmp;
	MockRNG rng;
	ASSERT_THROW(jmp.instant_switch(rng, 0, -1), std::domain_error);
	ASSERT_THROW(jmp.instant_switch(rng, 0, 2), std::domain_error);
}

TEST(JumpMarkovProcess, instant_switch) {
	MockJumpMarkovProcess jmp;
	MockRNG rng;
	ASSERT_EQ(-2, jmp.instant_switch(rng, -4, 0.2));
	ASSERT_EQ(4, jmp.instant_switch(rng, 4, 0.2));
}
