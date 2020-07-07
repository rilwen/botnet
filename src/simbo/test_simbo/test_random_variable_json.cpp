#include <gtest/gtest.h>
#include "simbo/random_variable_json.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(RandomVariableJson, sparse_discrete) {
	json j = "{\"type\":\"sparse_discrete\",\"params\":{\"weights\":[0.25,0.75],\"values\":[\"foo\",\"bar\"]}}"_json;
	auto abstract_rv = j.get<std::unique_ptr<RandomVariable<std::string>>>();
	ASSERT_NE(nullptr, abstract_rv);
	SparseDiscreteRandomVariable<std::string>& rv = dynamic_cast<SparseDiscreteRandomVariable<std::string>&>(*abstract_rv);
	ASSERT_EQ(std::vector<std::string>({ "foo", "bar" }), rv.get_values());
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(rv.get_values()[0], rv(rng));
	ASSERT_EQ(rv.get_values()[1], rv(rng));
}

TEST(RandomVariableJson, discrete) {
	json j = "{\"type\":\"discrete\",\"params\":{\"weights\":[0.25,0.75],\"min\":0}}"_json;
	auto abstract_rv = j.get<std::unique_ptr<RandomVariable<int>>>();
	ASSERT_NE(nullptr, abstract_rv);
	DiscreteRandomVariable& rv = dynamic_cast<DiscreteRandomVariable&>(*abstract_rv);
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(0, rv(rng));
	ASSERT_EQ(1, rv(rng));
}

TEST(RandomVariableJson, constant) {
	json j = "{\"type\":\"constant\",\"params\":{\"value\":0.45}}"_json;
	auto abstract_rv = j.get<std::unique_ptr<RandomVariable<double>>>();
	ASSERT_NE(nullptr, abstract_rv);
	ConstantRandomVariable<double>& rv = dynamic_cast<ConstantRandomVariable<double>&>(*abstract_rv);
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(0.45, rv(rng));
	ASSERT_EQ(0.45, rv(rng));
}

TEST(RandomVariableJson, constant_primitive) {
	json j = "0.45"_json;
	auto abstract_rv = j.get<std::unique_ptr<RandomVariable<double>>>();
	ASSERT_NE(nullptr, abstract_rv);
	ConstantRandomVariable<double>& rv = dynamic_cast<ConstantRandomVariable<double>&>(*abstract_rv);
	MockRNG rng;
	rng.add_uniform(0.24999);
	rng.add_uniform(0.25001);
	ASSERT_EQ(0.45, rv(rng));
	ASSERT_EQ(0.45, rv(rng));
}

TEST(RandomVariableJson, discrete_throws) {
	json j = "{\"type\":\"discrete\",\"params\":{\"weights\":[0.25,0.75],\"min\":\"a\"}}"_json;
	ASSERT_THROW(j.get<std::unique_ptr<RandomVariable<std::string>>>(), DeserialisationError);
}
