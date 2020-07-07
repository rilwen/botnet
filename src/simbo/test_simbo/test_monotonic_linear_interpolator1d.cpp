#include <gtest/gtest.h>
#include "simbo/monotonic_linear_interpolator1d.hpp"
#include "simbo/utils.hpp"

using namespace simbo;

// ASSERT_THROW gryzie się z templates.
typedef MonotonicLinearInterpolator1D<double, double> TestedClass;

TEST(MonotonicLinearInterpolator1D, constructor_throws) {
	std::vector<double> xs({ 0., 1, 4 });
	std::vector<double> ys({ 0., -0.5, 1 });
	ASSERT_THROW(TestedClass(make_copy(xs), std::move(ys)), std::invalid_argument);
	ys = { 0., 0.5, -1 };
	ASSERT_THROW(TestedClass(make_copy(xs), std::move(ys)), std::invalid_argument);
	std::vector<double> slopes({ -1, 1 });
	ASSERT_THROW(TestedClass(std::move(xs), std::move(slopes), -1), std::invalid_argument);
}

TEST(MonotonicLinearInterpolator1D, constructor_one_point) {
	std::vector<double> xs({ 1. });
	std::vector<double> ys({ -0.5 });
	TestedClass interp(std::move(xs), std::move(ys));
	ASSERT_EQ(1u, interp.size());
}

TEST(MonotonicLinearInterpolator1D, not_decreasing) {
	std::vector<double> xs({ 0., 1, 4, 5 });
	std::vector<double> ys({ 0., 0.1, 0.1, 0.2, });
	TestedClass interp(std::move(xs), std::move(ys));
	ASSERT_EQ(Monotonicity::NOT_DECREASING, interp.get_monotonicity());
	ASSERT_NEAR(0.15, interp(4.5), 1e-14);
	ASSERT_EQ(0., interp.invert_left(0.));
	ASSERT_EQ(0., interp.invert_right(0.));
	ASSERT_NEAR(0.25, interp.invert_left(0.025), 1e-14);
	ASSERT_NEAR(0.75, interp.invert_right(0.075), 1e-14);
	ASSERT_EQ(1., interp.invert_left(0.1));
	ASSERT_EQ(4., interp.invert_right(0.1));
	ASSERT_NEAR(4.9, interp.invert_left(0.19), 1e-14);
	ASSERT_NEAR(4.1, interp.invert_right(0.11), 1e-14);
	ASSERT_EQ(5, interp.invert_left(0.2));
	ASSERT_EQ(5, interp.invert_right(0.2));
}

TEST(MonotonicLinearInterpolator1D, non_negative) {
	std::vector<double> xs({ 0., 1, 4, 5 });
	const std::vector<double> slopes({ 0.1, 0., 0.1, });
	TestedClass interp(std::move(xs), slopes, 0.);
	ASSERT_EQ(Monotonicity::NOT_DECREASING, interp.get_monotonicity());
	ASSERT_NEAR(0.15, interp(4.5), 1e-14);
	ASSERT_EQ(0., interp.invert_left(0.));
	ASSERT_EQ(0., interp.invert_right(0.));
	ASSERT_NEAR(0.25, interp.invert_left(0.025), 1e-14);
	ASSERT_NEAR(0.75, interp.invert_right(0.075), 1e-14);
	ASSERT_EQ(1., interp.invert_left(0.1));
	ASSERT_EQ(4., interp.invert_right(0.1));
	ASSERT_NEAR(4.9, interp.invert_left(0.19), 1e-14);
	ASSERT_NEAR(4.1, interp.invert_right(0.11), 1e-14);
	ASSERT_EQ(5, interp.invert_left(0.2));
	ASSERT_EQ(5, interp.invert_right(0.2));
}

TEST(MonotonicLinearInterpolator1D, not_increasing) {
	std::vector<double> xs({ 0., 1, 4, 5 });
	std::vector<double> ys({ 0., -0.1, -0.1, -0.2, });
	TestedClass interp(std::move(xs), std::move(ys));
	ASSERT_EQ(Monotonicity::NOT_INCREASING, interp.get_monotonicity());
	ASSERT_NEAR(-0.15, interp(4.5), 1e-14);
	ASSERT_EQ(0., interp.invert_left(0.));
	ASSERT_EQ(0., interp.invert_right(0.));
	ASSERT_NEAR(0.25, interp.invert_left(-0.025), 1e-14);
	ASSERT_NEAR(0.75, interp.invert_right(-0.075), 1e-14);
	ASSERT_EQ(1., interp.invert_left(-0.1));
	ASSERT_EQ(4., interp.invert_right(-0.1));
	ASSERT_NEAR(4.9, interp.invert_left(-0.19), 1e-14);
	ASSERT_NEAR(4.1, interp.invert_right(-0.11), 1e-14);
	ASSERT_EQ(5, interp.invert_left(-0.2));
	ASSERT_EQ(5, interp.invert_right(-0.2));
}

TEST(MonotonicLinearInterpolator1D, non_positive) {
	std::vector<double> xs({ 0., 1, 4, 5 });
	const std::vector<double> slopes({ -0.1, 0., -0.1, });
	TestedClass interp(std::move(xs), slopes, 0.);
	ASSERT_EQ(Monotonicity::NOT_INCREASING, interp.get_monotonicity());
	ASSERT_NEAR(-0.15, interp(4.5), 1e-14);
	ASSERT_EQ(0., interp.invert_left(0.));
	ASSERT_EQ(0., interp.invert_right(0.));
	ASSERT_NEAR(0.25, interp.invert_left(-0.025), 1e-14);
	ASSERT_NEAR(0.75, interp.invert_right(-0.075), 1e-14);
	ASSERT_EQ(1., interp.invert_left(-0.1));
	ASSERT_EQ(4., interp.invert_right(-0.1));
	ASSERT_NEAR(4.9, interp.invert_left(-0.19), 1e-14);
	ASSERT_NEAR(4.1, interp.invert_right(-0.11), 1e-14);
	ASSERT_EQ(5, interp.invert_left(-0.2));
	ASSERT_EQ(5, interp.invert_right(-0.2));
}
