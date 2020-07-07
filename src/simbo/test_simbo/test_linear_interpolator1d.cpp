#include <gtest/gtest.h>
#include "simbo/linear_interpolator1d.hpp"
#include "simbo/utils.hpp"

using namespace simbo;

TEST(LinearInterpolator1D, evaluate) {
	const std::vector<double> xs({ 0, 1, 2 });
	const std::vector<double> ys({ 0.5, 0.6, 0.7 });
	LinearInterpolator1D<double, double> interp(make_copy(xs), make_copy(ys));
	for (int i = 0; i < xs.size(); ++i) {
		ASSERT_EQ(ys[i], interp.evaluate(xs[i], i)) << i;
	}
	ASSERT_NEAR(0.52, interp.evaluate(0.2, 0), 1e-15);
	ASSERT_NEAR(0.68, interp.evaluate(1.8, 1), 1e-15);
}

TEST(LinearInterpolator1D, integrate) {
	std::vector<double> xs({ 0, 1, 2 });
	const std::vector<double> slopes({ 0.5, 0.6, 0.7 });
	const double initial_value = 10;
	const LinearInterpolator1D<double, double> linear(std::move(xs), slopes, initial_value);
	ASSERT_EQ(3u, linear.size());
	ASSERT_EQ(initial_value, linear(0));
	ASSERT_NEAR(initial_value + 0.25, linear(0.5), 1e-14);
	ASSERT_NEAR(initial_value + 0.5, linear(1), 1e-14);
	ASSERT_NEAR(initial_value + 0.5 + 0.2 * 0.6, linear(1.2), 1e-14);
	ASSERT_NEAR(initial_value + 0.5 + 0.6, linear(2), 1e-14);
}
