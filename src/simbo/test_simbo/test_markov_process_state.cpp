#include <gtest/gtest.h>
#include "simbo/constant_process.hpp"
#include "simbo/markov_process_state.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef MarkovProcessState<int> TestedClass;

TEST(MarkovProcessState, constructor) {
	TestedClass st_false(1, 0.2, false);
	ASSERT_EQ(1, st_false.get_value());
	ASSERT_EQ(0.2, st_false.get_last_change_time());
	ASSERT_EQ(0.2, st_false.get_last_update_time());
	ASSERT_FALSE(st_false.is_by_process());
	TestedClass st_true(1, 0.3, true);
	ASSERT_EQ(0.3, st_true.get_last_change_time());
	ASSERT_EQ(0.3, st_true.get_last_update_time());
	ASSERT_TRUE(st_true.is_by_process());
}

TEST(MarkovProcessState, constructor_throws) {
	ASSERT_THROW(TestedClass(0, undefined_time(), false), std::domain_error);
}

TEST(MarkovProcessState, evolve_throws_wrong_value) {
	ConstantProcess<int> process(1);
	TestedClass state(0, 0., true);
	MockRNG rng;
	ASSERT_THROW(state.evolve(rng, process, 0.1), std::domain_error);
}

TEST(MarkovProcessState, evolve_throws_wrong_time) {
	ConstantProcess<int> process(1);
	TestedClass state(1, 0., true);
	MockRNG rng;
	ASSERT_THROW(state.evolve(rng, process, 0.), std::domain_error);
	ASSERT_THROW(state.evolve(rng, process, -0.1), std::domain_error);
}

TEST(MarkovProcessState, reset_throws) {
	TestedClass state(1, 0., true);
	ASSERT_THROW(state.reset(-1, -0.1), std::domain_error);
	ASSERT_THROW(state.reset(-1, 0.), std::domain_error);
}

TEST(MarkovProcessState, reset_later_time) {
	TestedClass state(1, 0., true);
	state.reset(-1, 0.1);
	ASSERT_EQ(-1, state.get_value());
	ASSERT_EQ(0.1, state.get_last_change_time());
	ASSERT_EQ(0.1, state.get_last_update_time());
	ASSERT_FALSE(state.is_by_process());
}

TEST(MarkovProcessState, evolve_by_process_true) {
	ConstantProcess<int> process(1);
	TestedClass state(1, 0., true);
	MockRNG rng;
	state.evolve(rng, process, 0.5);
	ASSERT_EQ(1, state.get_value());
	ASSERT_EQ(0.5, state.get_last_update_time());
	ASSERT_EQ(0., state.get_last_change_time());
	ASSERT_TRUE(state.is_by_process());
	state.evolve(rng, process, 1.5);
	ASSERT_EQ(1.5, state.get_last_update_time());
	ASSERT_EQ(0., state.get_last_change_time());
}

TEST(MarkovProcessState, evolve_by_process_false) {
	ConstantProcess<int> process(1);
	TestedClass state(0, 0., false);
	MockRNG rng;
	state.evolve(rng, process, 0.5);
	ASSERT_EQ(1, state.get_value());
	ASSERT_EQ(0.5, state.get_last_change_time());
	ASSERT_EQ(0.5, state.get_last_update_time());
	ASSERT_TRUE(state.is_by_process());
	state.evolve(rng, process, 1.5);
	ASSERT_EQ(0.5, state.get_last_change_time());
	ASSERT_EQ(1.5, state.get_last_update_time());
}
