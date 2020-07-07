#include <gtest/gtest.h>
#include "simbo/indexed_interpolator1d.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"

using namespace simbo;

TEST(IndexedInterpolator1D, constructor) {
	std::vector<std::string> values({ "ala", "ma", "kota" });
	std::vector<double> xs({ 0, 2, 3, 10, 11 });
	std::vector<int> ys({ 1, 0, 2, 3 });
	const size_t n = xs.size();
	const double x_f = xs.front();
	const double x_b = xs.back();
	const IndexedInterpolator1D<double, std::string> interp(std::move(values), std::make_unique<const PiecewiseConstantInterpolator1D<double, int>>(
		std::move(xs), std::move(ys)));
	ASSERT_EQ(n, interp.size());
	ASSERT_EQ(x_f, interp.get_lower_bound());
	ASSERT_EQ(x_b, interp.get_upper_bound());
}

typedef IndexedInterpolator1D<double, std::string> TestedClass;

TEST(IndexedInterpolator1D, constructor_throws) {
	ASSERT_THROW(TestedClass(std::vector<std::string>(), std::make_unique<const PiecewiseConstantInterpolator1D<double, int>>(
	std::vector<double>({ 0., 1. }), std::vector<int>({ 0, 1 }))), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<std::string>({ "aaaa" }), nullptr), std::invalid_argument);
}

TEST(IndexedInterpolator1D, find_left_node) {
	std::vector<std::string> values({ "ala", "ma", "kota" });
	std::vector<double> xs({ 0, 2, 3, 10, 11 });
	std::vector<int> ys({ 1, 0, 2, 3 });
	const IndexedInterpolator1D<double, std::string> interp(std::move(values), std::make_unique<const PiecewiseConstantInterpolator1D<double, int>>(
		std::move(xs), std::move(ys)));
	ASSERT_EQ(0, interp.find_left_node_index(0.));
	ASSERT_EQ(0, interp.find_left_node_index(1.1));
	ASSERT_EQ(1, interp.find_left_node_index(2.));
	ASSERT_EQ(1, interp.find_left_node_index(2.5));
	ASSERT_EQ(2, interp.find_left_node_index(3));
	ASSERT_EQ(2, interp.find_left_node_index(5.5));
	ASSERT_EQ(4, interp.find_left_node_index(11));
}

TEST(IndexedInterpolator1D, evaluate) {
	std::vector<std::string> values({ "ala", "ma", "kota" });
	std::vector<double> xs({ 0, 2, 3, 10, 11 });
	std::vector<int> ys({ 1, 0, 2, 3 });
	const IndexedInterpolator1D<double, std::string> interp(std::move(values), std::make_unique<const PiecewiseConstantInterpolator1D<double, int>>(
		std::move(xs), std::move(ys)));
	ASSERT_EQ("ma", interp.evaluate(0.5, 0));
	ASSERT_EQ("ala", interp.evaluate(2.5, 1));
	ASSERT_EQ("kota", interp.evaluate(10.5, 3));
}
