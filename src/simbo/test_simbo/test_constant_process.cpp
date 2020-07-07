#include <gtest/gtest.h>
#include "simbo/constant_process.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef ConstantProcess<double> TestedClass;

TEST(ConstantProcess, test) {
	const double v = 0.2;
	TestedClass process(v);
	MockRNG rng;
	ASSERT_EQ(v, process.get_next_state(rng, v, 10, 20));
	ASSERT_EQ(v, process.instant_switch(rng, v, 15));
	ASSERT_EQ(- infinite_time(), process.get_first_time());
	ASSERT_EQ(infinite_time(), process.get_last_time());
	ASSERT_THROW(process.get_next_state(rng, v + 1, 10, 20), std::domain_error);
}
