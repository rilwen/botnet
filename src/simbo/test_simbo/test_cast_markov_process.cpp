#include <gtest/gtest.h>
#include "simbo/activity_state.hpp"
#include "simbo/cast_markov_process.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/predetermined_process.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef CastMarkovProcess<ActivityState, int> TestedClass;

TEST(CastMarkovProcess, constructor_throws) {
	ASSERT_THROW(TestedClass(nullptr), std::invalid_argument);
}

TEST(CastMarkovProcess, test) {
	const seconds_t t0 = 0.5;
	const seconds_t t1 = 1.2;
	const seconds_t t2 = 2.4;
	const int i0 = 1;
	const int i1 = 0;
	std::vector<seconds_t> times({ t0, t1, t2 });
	std::vector<int> values({ i0, i1 });
	PredeterminedProcess<int>::values_interpolator_ptr interpolator(new PiecewiseConstantInterpolator1D<seconds_t, int>(std::move(times), std::move(values)));
	TestedClass::hidden_markov_process_ptr hidden_process(new PredeterminedProcess<int>(std::move(interpolator)));
	TestedClass process(std::move(hidden_process));
	MockRNG rng;
	ASSERT_EQ(t0, process.get_first_time());
	ASSERT_EQ(t2, process.get_last_time());
	ASSERT_EQ(ActivityState::ON, process.instant_switch(rng, ActivityState::OFF, t0));
	ASSERT_EQ(ActivityState::OFF, process.instant_switch(rng, ActivityState::ON, t1));
	ASSERT_EQ(ActivityState::OFF, process.get_next_state(rng, ActivityState::ON, t0, t2));
	ASSERT_THROW(process.get_next_state(rng, ActivityState::OFF, t0, t2), std::domain_error);
}
