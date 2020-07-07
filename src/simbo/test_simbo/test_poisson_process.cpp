#include <gtest/gtest.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/kurtosis.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/skewness.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include "simbo/monotonic_linear_interpolator1d.hpp"
#include "simbo/json.hpp"
#include "simbo/poisson_process.hpp"
#include "simbo/rng_impl.hpp"
#include "simbo/schedule.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;
using namespace boost::accumulators;

TEST(PoissonProcess, constructor) {
	const auto t0 = 0.1;
	const auto t1 = 2.5;
	std::vector<double> times({ t0, 0.2, 1.2, 1.3, t1 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	auto interp = PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.));
	const PoissonProcess pp(std::move(interp));
	ASSERT_EQ(t0, pp.get_first_time());
	ASSERT_EQ(t1, pp.get_last_time());
	ASSERT_EQ(nullptr, interp);
}

TEST(PoissonProcess, constructor_throws) {
	ASSERT_THROW(PoissonProcess(nullptr), std::invalid_argument);
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> bad_lambdas({ 0., -0.001, -2.0, -0.2 });
	auto bad_interp = PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(bad_lambdas), 0.));
	ASSERT_THROW(PoissonProcess(std::move(bad_interp)), std::invalid_argument);
	ASSERT_NE(nullptr, bad_interp);
}

TEST(PoissonProcess, get_next_jump) {
	MockRNG rng;
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	const PoissonProcess pp(PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.)));
	rng.add_uniform(1);
	rng.add_uniform(1);
	rng.add_uniform(0.99900049983338);
	rng.add_uniform(0.81791243155386);
	rng.add_uniform(0.64985891077475);
	rng.add_uniform(0.5);
	rng.add_uniform(0.81873075307798);
	rng.add_uniform(0.65050909472332);
	rng.add_uniform(0.001);
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 0, 0.1, 2.5));
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 0, 0.2, 2.5));
	ASSERT_NEAR(1.2, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 0, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 0, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(1.21, pp.get_next_jump(rng, 0, 1.2, 1.21), 1e-11);
}

TEST(PoissonProcess, get_next_state) {
	MockRNG rng;
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	const PoissonProcess pp(PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.)));
	rng.add_uniform(1.);
	rng.add_uniform(0.);
	rng.add_uniform(0.999999);
	rng.add_uniform(0.);
	rng.add_uniform(exp(-0.201) - 1e-3);
	rng.add_uniform(exp(-0.201) + 1e-3);
	rng.add_uniform(0.);
	ASSERT_EQ(0u, pp.get_next_state(rng, 0, 0.11, 0.19));
	ASSERT_EQ(0u, pp.get_next_state(rng, 0, 0.11, 0.19));
	ASSERT_EQ(1u, pp.get_next_state(rng, 0, 0.2, 1.2));
	ASSERT_EQ(0u, pp.get_next_state(rng, 0, 0.2, 1.3));
	ASSERT_EQ(1u, pp.get_next_state(rng, 0, 0.2, 1.3));
	ASSERT_THROW(pp.get_next_state(rng, 0, -0.1, 0.2), std::domain_error);
	ASSERT_THROW(pp.get_next_state(rng, 0, 0.11, 2.6), std::domain_error);
}

TEST(PoissonProcess, get_next_state_stats) {
	RNGImpl rng(42);
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	auto interp = PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.));
	const seconds_t t0 = 0.34;
	const seconds_t t1 = 2.22;
	const double h = (*interp)(t1) - (*interp)(t0);
	const PoissonProcess pp(std::move(interp));
	const double expected_mean = h;
	const double expected_variance = h;
	const double expected_skewness = 1. / sqrt(h);
	const double expected_excess_kurtosis = 1. / h;
	const double expected_median = floor(h + 1. / 3 - 0.002 / h);
	accumulator_set<double, stats<tag::mean, tag::variance, tag::kurtosis, tag::skewness, tag::median>> acc;
	const size_t n = 10000;
	for (size_t i = 0; i < n; ++i) {
		acc(static_cast<double>(pp.get_next_state(rng, 0, t0, t1)));
	}
	EXPECT_NEAR(expected_mean, mean(acc), 3 * sqrt(expected_variance / n));
	EXPECT_NEAR(expected_variance, variance(acc), 3 * sqrt(2 * expected_variance / n));
	EXPECT_NEAR(expected_skewness, skewness(acc), 1E-2);
	EXPECT_NEAR(expected_excess_kurtosis, kurtosis(acc), 2E-2);
	EXPECT_NEAR(expected_median, median(acc), 1E-3);
}

TEST(PoissonProcess, from_json_seasonal_weekly) {
	json j = "{\"workday_lambdas\":[0,1e-4,0,1e-4,0],\"weekend_lambdas\":[0,1e-4,0],\"workday_offsets\":[{\"hours\":8},{\"hours\":8,\"minutes\":30},{\"hours\":19},{\"hours\":21}],\"weekend_offsets\":[{\"hours\":10},{\"hours\":22}]}"_json;
	std::unique_ptr<PoissonProcess> process;
	const Schedule schedule(DateTime(Date(2018, 8, 3), TimeDuration()), TimeDuration(0, 30, 0, 0), 96);
	from_json_seasonal_weekly(j, schedule, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(0, process->get_first_time());
	ASSERT_EQ(48 * 3600, process->get_last_time());
	RNGImpl rng(25345345);
	int nbr_jumps = 0;
	seconds_t t0 = 24 * 3600;
	seconds_t t1 = 24 * 3600 + 1700;
	for (int i = 0; i < 100; ++i) {
		const auto t = process->get_next_jump(rng, 0, t0, t1);
		if (t < t1) {
			++nbr_jumps;
		}
	}
	ASSERT_EQ(0, nbr_jumps);
	nbr_jumps = 0;
	t0 = 8 * 3600;
	t1 = 8.5 * 3600;
	for (int i = 0; i < 100; ++i) {
		const auto t = process->get_next_jump(rng, 0, t0, t1);
		if (t < t1) {
			++nbr_jumps;
		}
	}
	ASSERT_GE(nbr_jumps, 0);
}

TEST(PoissonProcess, from_json_seasonal_weekly_no_offsets) {
	json j = "{\"workday_lambdas\":[1e-4],\"weekend_lambdas\":[0]}"_json;
	std::unique_ptr<PoissonProcess> process;
	const Schedule schedule(DateTime(Date(2018, 8, 3), TimeDuration()), TimeDuration(0, 30, 0, 0), 96);
	from_json_seasonal_weekly(j, schedule, process);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(0, process->get_first_time());
	ASSERT_EQ(48 * 3600, process->get_last_time());
}
