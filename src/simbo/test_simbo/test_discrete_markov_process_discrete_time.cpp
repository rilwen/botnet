#include <gtest/gtest.h>
#include "simbo/discrete_markov_process_discrete_time.hpp"
#include "simbo/indexed_interpolator1d.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;

enum class MockState {
	ONE,
	TWO
};

TEST(DiscreteMarkovProcessDiscreteTime, constructor) {
	MockRNG rng;
	const typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix pi({
		DiscreteRandomVariable(std::vector<double>({0.9, 0.1}), 0),
		DiscreteRandomVariable(std::vector<double>({0.4, 0.6}), 0)
	});
	std::vector<typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix> pis({ pi });
	const int first_period = -100;
	const int last_period = 100;
	auto index_interpolator = std::make_unique<const PiecewiseConstantInterpolator1D<int, int>>(std::vector<int>({ first_period, last_period }),
		std::vector<int>({ 0 }));
	DiscreteMarkovProcessDiscreteTime<MockState>::interpolator_ptr indexed_interpolator = std::make_unique<const IndexedInterpolator1D<int, typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix>>(std::move(pis), std::move(index_interpolator));
	const seconds_t time_zero = 10;
	const seconds_t period = 50;
	const DiscreteMarkovProcessDiscreteTime<MockState> dmp(std::move(indexed_interpolator), time_zero, period);
	ASSERT_NEAR(time_zero + first_period * period, dmp.get_first_time(), 1e-12);
	ASSERT_NEAR(time_zero + last_period * period, dmp.get_last_time(), 1e-12);
}

TEST(DiscreteMarkovProcessDiscreteTime, constructor_throws) {
	MockRNG rng;
	const typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix pi({
		DiscreteRandomVariable(std::vector<double>({ 0.9, 0.1 }), 0),
		DiscreteRandomVariable(std::vector<double>({ 0.4, 0.6 }), 0)
	});
	const std::vector<typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix> pis({ pi });
	const int first_period = -100;
	const int last_period = 100;
	auto good_index_interpolator = std::make_unique<const PiecewiseConstantInterpolator1D<int, int>>(std::vector<int>({ first_period, last_period }),
		std::vector<int>({ 0 }));
	DiscreteMarkovProcessDiscreteTime<MockState>::interpolator_ptr good_indexed_interpolator = std::make_unique<const IndexedInterpolator1D<int, typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix>>(make_copy(pis), std::move(good_index_interpolator));
	auto bad_index_interpolator = std::make_unique<const PiecewiseConstantInterpolator1D<int, int>>(std::vector<int>({ 1 }),
		std::vector<int>({ 0 }));
	DiscreteMarkovProcessDiscreteTime<MockState>::interpolator_ptr bad_indexed_interpolator = std::make_unique<const IndexedInterpolator1D<int, typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix>>(make_copy(pis), std::move(bad_index_interpolator));
	ASSERT_THROW(DiscreteMarkovProcessDiscreteTime<MockState>(nullptr, 10, 50), std::invalid_argument);
	good_index_interpolator = std::make_unique<const PiecewiseConstantInterpolator1D<int, int>>(std::vector<int>({ first_period, last_period }),
		std::vector<int>({ 0 }));
	good_indexed_interpolator = std::make_unique<const IndexedInterpolator1D<int, typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix>>(make_copy(pis), std::move(good_index_interpolator));
	ASSERT_THROW(DiscreteMarkovProcessDiscreteTime<MockState>(std::move(good_indexed_interpolator), 10, -50), std::domain_error);
	ASSERT_THROW(DiscreteMarkovProcessDiscreteTime<MockState>(std::move(bad_indexed_interpolator), 10, 50), std::invalid_argument);
}

void test_transitions(const DiscreteMarkovProcessDiscreteTime<MockState>& dmp, const std::vector<std::vector<double>>& pi_values, MockRNG& mock_rng, const seconds_t t0, const seconds_t t1) {
	mock_rng.clear();
	mock_rng.add_uniform(std::max(pi_values[0][0] - 1e-6, 0.));
	mock_rng.add_uniform(std::min(pi_values[0][0] + 1e-6, 1.));
	mock_rng.add_uniform(std::max(pi_values[1][0] - 1e-6, 0.));
	mock_rng.add_uniform(std::min(pi_values[1][0] + 1e-6, 1.));
	// Musimy wywołaść get_next_state dokładnie 4 razy.
	const auto next_11 = dmp.get_next_state(mock_rng, MockState::ONE, t0, t1);
	const auto next_12 = dmp.get_next_state(mock_rng, MockState::ONE, t0, t1);
	const auto next_21 = dmp.get_next_state(mock_rng, MockState::TWO, t0, t1);
	const auto next_22 = dmp.get_next_state(mock_rng, MockState::TWO, t0, t1);
	// P-stwo przejścia równe zero powinno je wykluczać zawsze.
	if (pi_values[0][0] > 0)
		ASSERT_EQ(MockState::ONE, next_11) << t0 << " -> " << t1;
	else
		ASSERT_EQ(MockState::TWO, next_11) << t0 << " -> " << t1;
	if (pi_values[0][1] > 0)
		ASSERT_EQ(MockState::TWO, next_12) << t0 << " -> " << t1;
	else
		ASSERT_EQ(MockState::ONE, next_12) << t0 << " -> " << t1;
	if (pi_values[1][0] > 0)
		ASSERT_EQ(MockState::ONE, next_21) << t0 << " -> " << t1;
	else
		ASSERT_EQ(MockState::TWO, next_21) << t0 << " -> " << t1;
	if (pi_values[1][1] > 0)
		ASSERT_EQ(MockState::TWO, next_22) << t0 << " -> " << t1;
	else
		ASSERT_EQ(MockState::ONE, next_22) << t0 << " -> " << t1;
}

TEST(DiscreteMarkovProcessDiscreteTime, test) {
	MockRNG mock_rng;
	const std::vector<std::vector<double>> id_values({
		std::vector<double>({ 1., 0. }),
		std::vector<double>({ 0., 1. })
	});
	const std::vector<std::vector<double>> pi0_values({
		std::vector<double>({ 0.9, 0.1 }),
		std::vector<double>({ 0.4, 0.6 })
	});
	const typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix pi0({
		DiscreteRandomVariable(pi0_values[0], 0),
		DiscreteRandomVariable(pi0_values[1], 0)
	});
	const std::vector<std::vector<double>> pi1_values({
		std::vector<double>({ 0.1, 0.9 }),
		std::vector<double>({ 0.6, 0.4 })
	});
	const typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix pi1({
		DiscreteRandomVariable(pi1_values[0], 0),
		DiscreteRandomVariable(pi1_values[1], 0)
	});
	std::vector<typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix> pis({ pi0, pi1 });
	const int first_period = -100;
	const int switch_period = 0;
	const int last_period = 100;
	auto index_interpolator = std::make_unique<const PiecewiseConstantInterpolator1D<int, int>>(std::vector<int>({ first_period, switch_period, last_period }),
		std::vector<int>({ 0, 1 }));
	DiscreteMarkovProcessDiscreteTime<MockState>::interpolator_ptr indexed_interpolator = std::make_unique<const IndexedInterpolator1D<int, typename DiscreteMarkovProcessDiscreteTime<MockState>::TransitionMatrix>>(std::move(pis), std::move(index_interpolator));
	const seconds_t time_zero = 10;
	const seconds_t period = 50;
	const DiscreteMarkovProcessDiscreteTime<MockState> dmp(std::move(indexed_interpolator), time_zero, period);
	test_transitions(dmp, pi0_values, mock_rng, time_zero + first_period * period, time_zero + (first_period + 1) * period);
	test_transitions(dmp, pi0_values, mock_rng, time_zero + first_period * period / 2, time_zero + (first_period / 2 + 1) * period);
	test_transitions(dmp, pi0_values, mock_rng, time_zero + (switch_period - 1) * period, time_zero + switch_period * period);
	test_transitions(dmp, id_values, mock_rng, time_zero + (switch_period - 0.5) * period, time_zero + (switch_period - 0.25) * period);
	test_transitions(dmp, pi0_values, mock_rng, time_zero + switch_period * period - 1e-2, time_zero + switch_period * period);
	test_transitions(dmp, pi0_values, mock_rng, time_zero + switch_period * period - 1e-2, time_zero + switch_period * period + 1e-2);
	test_transitions(dmp, id_values, mock_rng, time_zero + switch_period * period, time_zero + switch_period * period + 1e-2);
	test_transitions(dmp, pi1_values, mock_rng, time_zero + switch_period * period, time_zero + (switch_period + 1) * period);
	test_transitions(dmp, pi1_values, mock_rng, time_zero + last_period * period / 2, time_zero + (last_period / 2 + 1) * period);
	test_transitions(dmp, pi1_values, mock_rng, time_zero + (last_period - 1) * period, time_zero + last_period * period);

	ASSERT_EQ(MockState::ONE, dmp.instant_switch(mock_rng, MockState::ONE, switch_period));
	ASSERT_EQ(MockState::TWO, dmp.instant_switch(mock_rng, MockState::TWO, switch_period));
	ASSERT_THROW(dmp.instant_switch(mock_rng, MockState::TWO, -1e6), std::domain_error);
	ASSERT_THROW(dmp.instant_switch(mock_rng, MockState::TWO, 1e6), std::domain_error);
}
