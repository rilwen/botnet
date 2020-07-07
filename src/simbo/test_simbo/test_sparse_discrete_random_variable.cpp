#include <cmath>
#include <gtest/gtest.h>
#include "simbo/bot_state.hpp"
#include "simbo/json.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef SparseDiscreteRandomVariable<std::string> TestedClass;

TEST(SparseDiscreteRandomVariable, constructor) {
	std::vector<std::string> values({ "foo", "bar" });
	TestedClass rv(std::vector<double>({ 0.8, 1.2 }), make_copy(values));
	ASSERT_EQ(values, rv.get_values());
	ASSERT_EQ(2, rv.get_number_values());
}

TEST(SparseDiscreteRandomVariable, constructor_throws) {
	ASSERT_THROW(TestedClass(std::vector<double>({ 0.4, 0.6 }), std::vector<std::string>({ "foo", "bar", "baz" })), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<double>(), std::vector<std::string>()), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<double>({ 0.4, 0.6, -0.1 }), std::vector<std::string>({ "foo", "bar", "baz" })), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<double>({ 0.4, 0.6, nan("") }), std::vector<std::string>({ "foo", "bar", "baz" })), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<double>({ 0, 0, 0 }), std::vector<std::string>({ "foo", "bar", "baz" })), std::invalid_argument);	
}

TEST(SparseDiscreteRandomVariable, get_probability) {
	std::vector<std::string> values({ "foo", "bar" });
	TestedClass rv(std::vector<double>({ 0.8, 1.2 }), std::move(values));
	ASSERT_NEAR(0.4, rv.get_probability(0), 1e-15);
	ASSERT_NEAR(0.6, rv.get_probability(1), 1e-15);
	ASSERT_THROW(rv.get_probability(2), std::out_of_range);
}

TEST(SparseDiscreteRandomVariable, get_value) {
	std::vector<std::string> values({ "foo", "bar" });
	TestedClass rv(std::vector<double>({ 0.8, 1.2 }), std::move(values));
	ASSERT_EQ("foo", rv.get_value(0));
	ASSERT_EQ("bar", rv.get_value(1));
	ASSERT_THROW(rv.get_value(2), std::out_of_range);
}

TEST(SparseDiscreteRandomVariable, sampling) {
	std::vector<std::string> values({ "foo", "bar" });
	TestedClass rv(std::vector<double>({ 0.4, 0.6 }), std::move(values));
	MockRNG rng;
	rng.add_uniform(0.2);
	rng.add_uniform(0.8);
	ASSERT_EQ(rv.get_values()[0], rv(rng));
	ASSERT_EQ(rv.get_values()[1], rv(rng));
}

TEST(SparseDiscreteRandomVariable, from_json_string) {
	json j = "{\"weights\":[0.25,0.75],\"values\":[\"foo\",\"bar\"]}"_json;
	std::unique_ptr<SparseDiscreteRandomVariable<std::string>> rv = j;
	ASSERT_NE(nullptr, rv);
	ASSERT_EQ(std::vector<std::string>({"foo", "bar"}), rv->get_values());
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(rv->get_values()[0], (*rv)(rng));
	ASSERT_EQ(rv->get_values()[1], (*rv)(rng));
}

TEST(SparseDiscreteRandomVariable, from_json_enum) {
	json j = "{\"weights\":[0.25,0.75],\"values\":[\"WORKING\",\"DORMANT\"]}"_json;
	std::unique_ptr<SparseDiscreteRandomVariable<BotState>> rv = j;
	ASSERT_NE(nullptr, rv);
	ASSERT_EQ(std::vector<BotState>({ BotState::WORKING, BotState::DORMANT }), rv->get_values());
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(rv->get_values()[0], (*rv)(rng));
	ASSERT_EQ(rv->get_values()[1], (*rv)(rng));
}
