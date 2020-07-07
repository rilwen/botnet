#include <gtest/gtest.h>
#include "simbo/json.hpp"
#include "simbo/monotonic_linear_interpolator1d.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/poisson_process.hpp"
#include "simbo/schedule.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class PausingJumpMarkovProcessTest : public ::testing::Test {
protected:
	PausingJumpMarkovProcessTest() {
		t0 = 0.5;
		t1 = 2.4;
		t2 = 3.1;
		lambda0 = 0.2;
		lambda1 = 0.001;
		delay = 0.2;
	}

	seconds_t t0;
	seconds_t t1;
	seconds_t t2;
	double lambda0;
	double lambda1;
	MockRNG rng;
	double delay;

	std::unique_ptr<const JumpMarkovProcess> make_base_jump_process() {
		std::vector<seconds_t> times({ t0, t1, t2 });
		std::vector<double> lambdas({ lambda0, lambda1, lambda1 });
		PoissonProcess::interpolator1d_ptr interpolator(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0));
		return std::make_unique<PoissonProcess>(std::move(interpolator));
	}
};

TEST_F(PausingJumpMarkovProcessTest, constructor) {
	PausingJumpMarkovProcess process(make_base_jump_process(), delay);
	ASSERT_EQ(t0, process.get_first_time());
	ASSERT_EQ(t2, process.get_last_time());
	ASSERT_EQ(delay, process.get_pause_length());
}

TEST_F(PausingJumpMarkovProcessTest, constructor_simple) {
	PausingJumpMarkovProcess process(2 / delay, delay);
	ASSERT_EQ(-infinite_time(), process.get_first_time());
	ASSERT_EQ(infinite_time(), process.get_last_time());
	ASSERT_EQ(delay, process.get_pause_length());
}

TEST_F(PausingJumpMarkovProcessTest, constructor_throws) {
	ASSERT_THROW(PausingJumpMarkovProcess(nullptr, 0.1), std::invalid_argument);
	ASSERT_THROW(PausingJumpMarkovProcess(make_base_jump_process(), -0.1), std::domain_error);
}

TEST_F(PausingJumpMarkovProcessTest, instant_switch) {
	PausingJumpMarkovProcess process(make_base_jump_process(), delay);
	const int i = 2;
	const auto actual = process.instant_switch(rng, std::make_pair(i, t1), t2);
	ASSERT_EQ(std::make_pair(i, t2 - delay), actual);
}

TEST_F(PausingJumpMarkovProcessTest, get_next_jump) {
	const auto base = make_base_jump_process();
	PausingJumpMarkovProcess process(make_base_jump_process(), delay);
	rng.add_uniform(exp(-lambda0 * (t1 - t0 - delay / 2)));
	const double t = t1 + 0.4;
	rng.add_uniform(exp(-lambda1 * (t - t1 - delay)));
	auto actual = process.get_next_state(rng, std::make_pair(0, t0 - delay / 2), t0, t2);
	auto expected = std::make_pair(2, t);
	ASSERT_EQ(expected.first, actual.first) << actual.first << " " << actual.second;
	ASSERT_NEAR(expected.second, actual.second, 1e-12);
}

TEST_F(PausingJumpMarkovProcessTest, from_json) {
	const DateTime start(Date(2016, 4, 20), TimeDuration(18, 30, 0, 0));
	const Schedule schedule(start);	
	json j = "{\"pause_length\": {\"hours\": 24},"
		"\"base_process\":{\"type\": \"poisson\", \"subtype\": \"simple\", \"params\": {\"inverse_lambda\": 3600}}"
		"}"_json;
	std::unique_ptr<PausingJumpMarkovProcess> process;
	from_json(j, schedule, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(24 * 3600, process->get_pause_length());
}