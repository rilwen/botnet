#include <gtest/gtest.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include "simbo/json.hpp"
#include "simbo/rng_impl.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;
using namespace boost::accumulators;

TEST(SimplePoissonProcess, constructor) {
	const SimplePoissonProcess pp(0.1);
	ASSERT_EQ(-infinite_time(), pp.get_first_time());
	ASSERT_EQ(infinite_time(), pp.get_last_time());
	ASSERT_EQ(0.1, pp.get_lambda());
}

TEST(SimplePoissonProcess, constructor_throws) {
	ASSERT_THROW(SimplePoissonProcess(-0.2), std::domain_error);
}

TEST(SimplePoissonProcess, get_next_jump) {
	MockRNG rng;
	const double lambda = 0.1;
	const SimplePoissonProcess pp(lambda);
	const double t = 1.2;
	rng.add_uniform(1);
	rng.add_uniform(exp(-t * lambda));
	rng.add_uniform(exp(-t * lambda));
	rng.add_uniform(0);
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 0, 0.2, 2.5));
	ASSERT_NEAR(t, pp.get_next_jump(rng, 0, 0, t + 1e-5), 1e-12);
	ASSERT_EQ(t / 2, pp.get_next_jump(rng, 0, 0, t / 2));
	ASSERT_EQ(2 * t, pp.get_next_jump(rng, 0, 0, 2 * t));
}

TEST(SimplePoissonProcess, get_next_state_stats) {
	RNGImpl rng(42);
	const double lambda = 0.6;
	const seconds_t t0 = 0.34;
	const seconds_t t1 = 2.22;
	const double h = (t1 - t0) * lambda;
	const SimplePoissonProcess pp(lambda);
	const double expected_mean = h;
	const double expected_variance = h;
	accumulator_set<double, stats<tag::mean, tag::variance>> acc;
	const size_t n = 10000;
	for (size_t i = 0; i < n; ++i) {
		acc(static_cast<double>(pp.get_next_state(rng, 0, t0, t1)));
	}
	EXPECT_NEAR(expected_mean, mean(acc), 3 * sqrt(expected_variance / n));
	EXPECT_NEAR(expected_variance, variance(acc), 3 * sqrt(2 * expected_variance / n));	
}

TEST(SimplePoissonProcess, from_json_lambda) {
	json j = "{\"lambda\":0.1}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	from_json(j, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(0.1, process->get_lambda());
}

TEST(SimplePoissonProcess, from_json_inverse_lambda) {
	json j = "{\"inverse_lambda\":10}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	from_json(j, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(0.1, process->get_lambda());
}

TEST(SimplePoissonProcess, from_json_inverse_lambda_time_duration) {
	json j = "{\"inverse_lambda\":{\"hours\": 1}}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	from_json(j, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(1 / 3600., process->get_lambda());
}

TEST(SimplePoissonProcess, from_json_throws) {
	json j = "{\"lambda\":-0.1}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	ASSERT_THROW(from_json(j, process), DeserialisationError);
}

TEST(SimplePoissonProcess, from_json_both_params_throws) {
	json j = "{\"lambda\":0.1,\"inverse_lambda\":0.1}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	ASSERT_THROW(from_json(j, process), DeserialisationError);
}

TEST(SimplePoissonProcess, from_json_no_params_throws) {
	json j = "{}"_json;
	std::unique_ptr<SimplePoissonProcess> process;
	ASSERT_THROW(from_json(j, process), DeserialisationError);
}