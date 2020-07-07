#include <gtest/gtest.h>
#include "simbo/interpolator1d_impl.hpp"
#include "simbo/utils.hpp"

using namespace simbo;

class MockInterpolator1DImpl : public Interpolator1DImpl<double, double> {
public:
	using Interpolator1DImpl<double, double>::Interpolator1DImpl;

	double evaluate(double x, int left_node_index) const override {
		return x + static_cast<double>(left_node_index);
	}
};

TEST(Interpolator1DImpl, constructor) {
	const std::vector<double> xs({ 0, 1, 2 });
	const std::vector<double> ys({ 0.5, 0.6, 0.7 });
	MockInterpolator1DImpl interp(make_copy(xs), make_copy(ys));
	ASSERT_EQ(xs.size(), interp.size());
	ASSERT_EQ(xs.front(), interp.get_lower_bound());
	ASSERT_EQ(xs.back(), interp.get_upper_bound());
	ASSERT_EQ(xs, interp.get_xs());
	ASSERT_EQ(ys, interp.get_ys());
}

TEST(Interpolator1DImpl, constructor_one_node) {
	std::vector<double> xs({ 0 });
	std::vector<double> ys({ 0.5 });
	MockInterpolator1DImpl interp(std::move(xs), std::move(ys));
	ASSERT_EQ(1, interp.size());
	ASSERT_EQ(0, interp.get_lower_bound());
	ASSERT_EQ(0, interp.get_upper_bound());	
}

TEST(Interpolator1DImpl, constructor_not_increasing_xs) {
	std::vector<double> xs({ 0, -1, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	ASSERT_THROW(MockInterpolator1DImpl(std::move(xs), std::move(ys)), std::invalid_argument);
}

TEST(Interpolator1DImpl, constructor_unequal_size) {
	std::vector<double> xs({ 0, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	ASSERT_THROW(MockInterpolator1DImpl(std::move(xs), std::move(ys)), std::invalid_argument);
}

TEST(Interpolator1DImpl, constructor_empty_args) {
	std::vector<double> xs;
	std::vector<double> ys;
	ASSERT_THROW(MockInterpolator1DImpl(std::move(xs), std::move(ys)), std::invalid_argument);
}

TEST(Interpolator1DImpl, find_left_node_index) {
	const std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	MockInterpolator1DImpl interp(make_copy(xs), std::move(ys));
	for (size_t i = 0; i < xs.size(); ++i) {
		ASSERT_EQ(i, interp.find_left_node_index(xs[i])) << i;
	}
	ASSERT_EQ(0, interp.find_left_node_index(0.5));
	ASSERT_EQ(1, interp.find_left_node_index(1.5));
	ASSERT_THROW(interp.find_left_node_index(-0.1), std::domain_error);
	ASSERT_THROW(interp.find_left_node_index(2.1), std::domain_error);
}

// ! Testuje kod zdefiniowany w klasie Interpolator1D.
TEST(Interpolator1D, operator_real) {
	std::vector<double> xs({ 0, 1, 2 });
	std::vector<double> ys({ 0.5, 0.6, 0.7 });
	MockInterpolator1DImpl interp(std::move(xs), std::move(ys));
	ASSERT_NEAR(0.23, interp(0.23), 1e-15);
	ASSERT_NEAR(2.23, interp(1.23), 1e-15);
}
