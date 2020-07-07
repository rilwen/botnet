#include <gtest/gtest.h>
#include "simbo/simple_autoscaled_poisson_process.hpp"
#include "simbo/rng_impl.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(SimpleAutoscaledPoissonProcess, constructor) {
	const double lambda = 0.4;
	const SimpleAutoscaledPoissonProcess pp(lambda);
	ASSERT_EQ(-infinite_time(), pp.get_first_time());
	ASSERT_EQ(infinite_time(), pp.get_last_time());
	ASSERT_EQ(lambda, pp.get_lambda());
}

TEST(SimpleAutoscaledPoissonProcess, constructor_throws) {
	ASSERT_THROW(SimpleAutoscaledPoissonProcess(-1), std::domain_error);
}

TEST(SimpleAutoscaledPoissonProcess, get_next_jump) {
	MockRNG rng;
	const double lambda = 2;
	const SimpleAutoscaledPoissonProcess pp(lambda);
	const double t0 = 0.12;
	const double t1 = 0.87;
	const double dt = t1 - t0;
	int prev_state = 1;
	rng.add_uniform(1);
	rng.add_uniform(exp(-prev_state * dt * lambda));
	rng.add_uniform(exp(-prev_state * dt * lambda));
	rng.add_uniform(0);
	ASSERT_EQ(t0, pp.get_next_jump(rng, prev_state, t0, t1));
	ASSERT_NEAR(t1, pp.get_next_jump(rng, prev_state, t0, t1 + 0.2), 1e-11);
	ASSERT_EQ(t0 + dt / 2, pp.get_next_jump(rng, prev_state, t0, t0 + dt / 2));
	ASSERT_EQ(t1, pp.get_next_jump(rng, prev_state, t0, t1));
	rng.reset();
	prev_state = 0;
	ASSERT_EQ(t1, pp.get_next_jump(rng, prev_state, t0, t1));
	ASSERT_NEAR(t1 + 0.2, pp.get_next_jump(rng, prev_state, t0, t1 + 0.2), 1e-11);
	ASSERT_EQ(t0 + dt / 2, pp.get_next_jump(rng, prev_state, t0, t0 + dt / 2));
	ASSERT_EQ(t1, pp.get_next_jump(rng, prev_state, t0, t1));
	rng.clear();
	prev_state = 2;
	rng.add_uniform(1);
	rng.add_uniform(exp(-prev_state * dt * lambda));
	rng.add_uniform(exp(-prev_state * dt * lambda));
	rng.add_uniform(0);
	ASSERT_EQ(t0, pp.get_next_jump(rng, prev_state, t0, t1));
	ASSERT_NEAR(t1, pp.get_next_jump(rng, prev_state, t0, t1 + 0.2), 1e-11);
	ASSERT_EQ(t0 + dt / 2, pp.get_next_jump(rng, prev_state, t0, t0 + dt / 2));
	ASSERT_EQ(t1, pp.get_next_jump(rng, prev_state, t0, t1));
}
