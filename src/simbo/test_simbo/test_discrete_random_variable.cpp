#include <cmath>
#include <gtest/gtest.h>
#include "simbo/discrete_random_variable.hpp"
#include "simbo/json.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(DiscreteRandomVariable, constructor) {
	const DiscreteRandomVariable rv(std::vector<double>({ 0.8, 1, 0.2 }), -1);
	ASSERT_EQ(-1, rv.min());
	ASSERT_EQ(1, rv.max());
	ASSERT_NEAR(0.4, rv.get_probability(-1), 1e-15);
	ASSERT_NEAR(0.5, rv.get_probability(0), 1e-15);
	ASSERT_NEAR(0.1, rv.get_probability(1), 1e-15);
}

TEST(DiscreteRandomVariable, copy_constructor) {
	const DiscreteRandomVariable rv(std::vector<double>({ 0.4, 0.5, 0.1 }), -1);
	DiscreteRandomVariable rv2(rv);
	ASSERT_EQ(rv.min(), rv2.min());
	ASSERT_EQ(rv.max(), rv2.max());
}

TEST(DiscreteRandomVariable, move_constructor) {
	DiscreteRandomVariable rv(std::vector<double>({ 0.4, 0.5, 0.1 }), -1);
	DiscreteRandomVariable rv2(std::move(rv));
	ASSERT_EQ(rv.min(), rv2.min());
	ASSERT_EQ(rv.max(), rv2.max());
}

TEST(DiscreteRandomVariable, constructor_throws) {
	ASSERT_THROW(DiscreteRandomVariable(std::vector<double>(), 0), std::invalid_argument);
	ASSERT_THROW(DiscreteRandomVariable(std::vector<double>({ 0 }), 0), std::invalid_argument);
	ASSERT_THROW(DiscreteRandomVariable(std::vector<double>({ 0.1, -0.2 }), 0), std::invalid_argument);
	ASSERT_THROW(DiscreteRandomVariable(std::vector<double>({ 0.1, nan("") }), 0), std::invalid_argument);
}

TEST(DiscreteRandomVariable, sample) {
	const DiscreteRandomVariable rv(std::vector<double>({ 0.6, 0.6, 0.8 }), -1);
	MockRNG rng;
	rng.add_uniform(0.3);
	rng.add_uniform(0.15);
	rng.add_uniform(0.6);
	rng.add_uniform(0.45);
	rng.add_uniform(0.75);
	rng.add_uniform(1);
	rng.add_uniform(0);
	ASSERT_EQ(0, rv(rng));
	ASSERT_EQ(-1, rv(rng));
	ASSERT_EQ(1, rv(rng));
	ASSERT_EQ(0, rv(rng));
	ASSERT_EQ(1, rv(rng));
	ASSERT_EQ(1, rv(rng));
	ASSERT_EQ(-1, rv(rng));
}

TEST(DiscreteRandomVariable, sample_certain) {
	const DiscreteRandomVariable rv(std::vector<double>({ 0., 2., 0. }), -1);
	MockRNG rng;
	rng.add_uniform(0.);
	rng.add_uniform(0.15);
	rng.add_uniform(1.);
	ASSERT_EQ(0, rv(rng));
	ASSERT_EQ(0, rv(rng));
	ASSERT_EQ(0, rv(rng));
}

TEST(DiscreteRandomVariable, from_json) {
	json j = "{\"weights\":[0.8,0,0.8],\"min\":-1}"_json;
	std::unique_ptr<DiscreteRandomVariable> rv = j;
	ASSERT_NE(nullptr, rv);
	ASSERT_EQ(-1, rv->min());
	ASSERT_EQ(1, rv->max());
	MockRNG rng;
	rng.add_uniform(0.49999);
	rng.add_uniform(0.50001);
	ASSERT_EQ(-1, (*rv)(rng));
	ASSERT_EQ(1, (*rv)(rng));
}