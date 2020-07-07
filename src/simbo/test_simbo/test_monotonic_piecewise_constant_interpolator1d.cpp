#include <gtest/gtest.h>
#include "simbo/interpolators_json.hpp"
#include "simbo/monotonic_piecewise_constant_interpolator1d.hpp"

using namespace simbo;

TEST(MonotonicPiecewiseConstantInterpolator1D, constructor) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(Monotonicity::STRICTLY_INCREASING, interp.get_monotonicity());
}

TEST(MonotonicPiecewiseConstantInterpolator1D, constructor_one_node) {
	std::vector<double> xs({ 1 });
	std::vector<double> ys({ 0.5 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(Monotonicity::CONSTANT, interp.get_monotonicity());
}

TEST(MonotonicPiecewiseConstantInterpolator1D, evaluate) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.5, interp.evaluate(0, 0));
	ASSERT_EQ(0.5, interp.evaluate(0.5, 0));
	ASSERT_EQ(0.6, interp(1));
	ASSERT_EQ(0.6, interp.evaluate(1.5, 1));
	ASSERT_EQ(0.6, interp(2));
}

TEST(MonotonicPiecewiseConstantInterpolator1D, new_value_right_edge) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.7, interp(2));
}

TEST(MonotonicPiecewiseConstantInterpolator1D, increasing) {
	std::vector<double> xs({ 0, 1, 2, 3 });
	std::vector<double> ys({ 0.2, 0.3, 0.3, 0.5 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.3, interp(1));
	ASSERT_EQ(0.2, interp(0.5));
}

TEST(MonotonicPiecewiseConstantInterpolator1D, decreasing) {
	std::vector<double> xs({ 0, 1, 2, 3 });
	std::vector<double> ys({ 0.5, 0.3, 0.3, 0.2 });
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.3, interp(1));
	ASSERT_EQ(0.5, interp(0.5));
}

TEST(MonotonicPiecewiseConstantInterpolator1D, from_json) {
	json j = "{\"xs\":[-1, 0, 1.5],"
		"\"ys\":[0.4, -0.7, -1.1]}"_json;
	MonotonicPiecewiseConstantInterpolator1D<double, double> interp = j;
	ASSERT_EQ(std::vector<double>({ -1, 0, 1.5 }), interp.get_xs());
	ASSERT_EQ(std::vector<double>({ 0.4, -0.7, -1.1 }), interp.get_ys());
}