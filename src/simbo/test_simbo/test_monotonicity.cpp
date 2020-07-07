#include <vector>
#include <gtest/gtest.h>
#include <simbo/monotonicity.hpp>

using namespace simbo;

TEST(Monotonicity, NotDecreasing) {
	const std::vector<int> x({ 0, 0, 1, 1, 2, 3, 4 });
	ASSERT_EQ(Monotonicity::NOT_DECREASING, check_monotonicity(x));
}

TEST(Monotonicity, StrictlyIncreasing) {
	const std::vector<double> x({ -1, 0, 1.5 });
	ASSERT_EQ(Monotonicity::STRICTLY_INCREASING, check_monotonicity(x));
}

TEST(Monotonicity, NotIncreasing) {
	const std::vector<int> x({ 10, 8, 3, 3, 0, 0, 0, -1, -3 });
	ASSERT_EQ(Monotonicity::NOT_INCREASING, check_monotonicity(x));
}

TEST(Monotonicity, StrictlyDecreasing) {
	const std::vector<int> x({ 10, 8, 3, 0, -1, -3 });
	ASSERT_EQ(Monotonicity::STRICTLY_DECREASING, check_monotonicity(x));
}

TEST(Monotonicity, Constant) {
	const std::vector<int> x({ 1, 1, 1, 1, 1 });
	ASSERT_EQ(Monotonicity::CONSTANT, check_monotonicity(x));
}

TEST(Monotonicity, None) {
	const std::vector<int> x({ 0, 0, 10, 10, 8, 7, 4, 1 });
	ASSERT_EQ(Monotonicity::NONE, check_monotonicity(x));
}

TEST(Monotonicity, OneValue) {
	const std::vector<int> x({ 1 });
	ASSERT_EQ(Monotonicity::CONSTANT, check_monotonicity(x));
}