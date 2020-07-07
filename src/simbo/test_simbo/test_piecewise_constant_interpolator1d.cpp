#include <gtest/gtest.h>
#include "simbo/interpolators_json.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"

using namespace simbo;

TEST(PiecewiseConstantInterpolator1D, evaluate) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6 });
	PiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.5, interp.evaluate(0.5, 0));
	ASSERT_EQ(0.6, interp.evaluate(1.5, 1));
	ASSERT_EQ(0.5, interp(0));
	ASSERT_EQ(0.6, interp(1));
	ASSERT_EQ(0.6, interp(2));
}

TEST(PiecewiseConstantInterpolator1D, new_value_right_edge) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	PiecewiseConstantInterpolator1D<double, double> interp(std::move(xs), std::move(ys));
	ASSERT_EQ(0.7, interp(2));
}

TEST(PiecewiseConstantInterpolator1D, from_json) {	
	json j = "{\"xs\":[-1, 0, 1.5],"
		"\"ys\":[0.4, -0.7, 0.1]}"_json;
	PiecewiseConstantInterpolator1D<double, double> interp = j;
	ASSERT_EQ(std::vector<double>({ -1, 0, 1.5 }), interp.get_xs());
	ASSERT_EQ(std::vector<double>({ 0.4, -0.7, 0.1 }), interp.get_ys());
}