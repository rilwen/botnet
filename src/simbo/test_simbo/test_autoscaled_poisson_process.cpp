#include <gtest/gtest.h>
#include "simbo/monotonic_linear_interpolator1d.hpp"
#include "simbo/autoscaled_poisson_process.hpp"
#include "simbo/rng_impl.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(AutoscaledPoissonProcess, constructor) {
	const auto t0 = 0.1;
	const auto t1 = 2.5;
	std::vector<double> times({ t0, 0.2, 1.2, 1.3, t1 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	auto interp = AutoscaledPoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.));
	const AutoscaledPoissonProcess pp(std::move(interp));
	ASSERT_EQ(t0, pp.get_first_time());
	ASSERT_EQ(t1, pp.get_last_time());
	ASSERT_EQ(nullptr, interp);
}

TEST(AutoscaledPoissonProcess, constructor_throws) {
	ASSERT_THROW(AutoscaledPoissonProcess(nullptr), std::invalid_argument);
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> bad_lambdas({ 0., -0.001, -2.0, -0.2 });
	auto bad_interp = AutoscaledPoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(bad_lambdas), 0.));
	ASSERT_THROW(AutoscaledPoissonProcess(std::move(bad_interp)), std::invalid_argument);
	ASSERT_NE(nullptr, bad_interp);
}

TEST(AutoscaledPoissonProcess, get_next_jump) {
	MockRNG rng;
	std::vector<double> times({ 0.1, 0.2, 1.2, 1.3, 2.5 });
	std::vector<double> lambdas({ 0., 0.001, 2.0, 0.2 });
	const AutoscaledPoissonProcess pp(AutoscaledPoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(std::move(times), std::move(lambdas), 0.)));
	rng.add_uniform(1);
	rng.add_uniform(1);
	rng.add_uniform(0.99900049983338);
	rng.add_uniform(0.81791243155386);
	rng.add_uniform(0.64985891077475);
	rng.add_uniform(0.5);
	rng.add_uniform(0.81873075307798);
	rng.add_uniform(0.65050909472332);
	rng.add_uniform(0.001);
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 1, 0.1, 2.5));
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 1, 0.2, 2.5));
	ASSERT_NEAR(1.2, pp.get_next_jump(rng, 1, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 1, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 1, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 1, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 1, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 1, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(1.21, pp.get_next_jump(rng, 1, 1.2, 1.21), 1e-11);
	rng.reset();
	ASSERT_EQ(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5));
	ASSERT_EQ(2.5, pp.get_next_jump(rng, 0, 0.2, 2.5));
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 0, 1.2, 2.5), 1e-11);
	rng.clear();
	rng.add_uniform(1);
	rng.add_uniform(1);
	rng.add_uniform(0.99900049983338 * 0.99900049983338);
	rng.add_uniform(0.81791243155386 * 0.81791243155386);
	rng.add_uniform(0.64985891077475 * 0.64985891077475);
	rng.add_uniform(0.25);
	rng.add_uniform(0.81873075307798 * 0.81873075307798);
	rng.add_uniform(0.65050909472332 * 0.65050909472332);
	rng.add_uniform(1e-6);
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 2, 0.1, 2.5));
	ASSERT_EQ(0.2, pp.get_next_jump(rng, 2, 0.2, 2.5));
	ASSERT_NEAR(1.2, pp.get_next_jump(rng, 2, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 2, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 2, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(2.5, pp.get_next_jump(rng, 2, 0.1, 2.5), 1e-11);
	ASSERT_NEAR(1.3, pp.get_next_jump(rng, 2, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(2.45, pp.get_next_jump(rng, 2, 1.2, 2.5), 1e-11);
	ASSERT_NEAR(1.21, pp.get_next_jump(rng, 2, 1.2, 1.21), 1e-11);
}
