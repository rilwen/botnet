#include <gtest/gtest.h>
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/predetermined_process.hpp"
#include "simbo/markov_switcher.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef MarkovSwitcher<std::string> TestedClass;

class MarkovSwitcherTest : public ::testing::Test {
protected:
	MarkovSwitcherTest() {
		t0 = 0.5;
		t1 = 1.2;
		t2 = 2.4;
		i0 = 1;
		i1 = 0;
		std::vector<seconds_t> times({ t0, t1, t2 });
		std::vector<int> values({ i0, i1 });
		PredeterminedProcess<int>::values_interpolator_ptr interpolator(new PiecewiseConstantInterpolator1D<seconds_t, int>(make_copy(times), std::move(values)));
		switching_process.reset(new PredeterminedProcess<int>(std::move(interpolator)));
		values = { 1, -1 };
		interpolator = PredeterminedProcess<int>::values_interpolator_ptr(new PiecewiseConstantInterpolator1D<seconds_t, int>(make_copy(times), std::move(values)));
		bad_switching_process.reset(new PredeterminedProcess<int>(std::move(interpolator)));
		good_values = { std::string("ala"), std::string("ma kota") };
		bad_values = { std::string("foo") };
	}

	seconds_t t0;
	seconds_t t1;
	seconds_t t2;
	int i0;
	int i1;
	std::vector<std::string> good_values;
	std::vector<std::string> bad_values;
	TestedClass::switching_process_ptr switching_process;
	TestedClass::switching_process_ptr bad_switching_process;
	MockRNG rng;
};

TEST_F(MarkovSwitcherTest, constructor) {
	TestedClass switcher(switching_process, make_copy(good_values), good_values[0]);
	ASSERT_EQ(good_values[0], switcher.get_current_value());
	ASSERT_EQ(-infinite_time(), switcher.get_last_change_time());
	ASSERT_EQ(-infinite_time(), switcher.get_last_update_time());
	ASSERT_EQ(good_values, switcher.get_values());
}

TEST_F(MarkovSwitcherTest, constructor_throws) {
	ASSERT_THROW(TestedClass(switching_process, TestedClass::values_vec(), ""), std::invalid_argument);
	ASSERT_THROW(TestedClass(nullptr, make_copy(good_values), ""), std::invalid_argument);
	ASSERT_THROW(TestedClass(switching_process, make_copy(good_values), "foo"), std::invalid_argument);
}

TEST_F(MarkovSwitcherTest, update) {
	TestedClass switcher(switching_process, make_copy(good_values), good_values[0]);
	switcher.update(rng, t0);
	ASSERT_EQ(good_values[i0], switcher.get_current_value());
	ASSERT_EQ(t0, switcher.get_last_change_time());
	ASSERT_EQ(t0, switcher.get_last_update_time());
	switcher.update(rng, (t0 + t1) / 2);
	ASSERT_EQ(good_values[i0], switcher.get_current_value());
	ASSERT_EQ(t0, switcher.get_last_change_time());
	ASSERT_EQ((t0 + t1) / 2, switcher.get_last_update_time());
	switcher.update(rng, t1);
	ASSERT_EQ(good_values[i1], switcher.get_current_value());
	ASSERT_EQ(t1, switcher.get_last_change_time());
	ASSERT_EQ(t1, switcher.get_last_update_time());
	switcher.update(rng, t2);
	ASSERT_EQ(good_values[i1], switcher.get_current_value());
	ASSERT_EQ(t1, switcher.get_last_change_time());
	ASSERT_EQ(t2, switcher.get_last_update_time());
}

TEST_F(MarkovSwitcherTest, update_throws) {
	TestedClass switcher(bad_switching_process, make_copy(bad_values), bad_values[0]);
	ASSERT_THROW(switcher.update(rng, t0 - 1), std::domain_error);
	ASSERT_THROW(switcher.update(rng, t2 + 1), std::domain_error);
	ASSERT_THROW(switcher.update(rng, t0), std::out_of_range);
	ASSERT_THROW(switcher.update(rng, t2), std::out_of_range);
}
