#include <gtest/gtest.h>
#include "simbo/brownian_motion.hpp"
#include "simbo/linear_interpolator1d.hpp"
#include "simbo/rng_impl.hpp"

using namespace simbo;

TEST(BrownianMotion, constructor) {
	auto vol_sqr = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ 0, 1.2 }),
		std::vector<double>({ 0., 100. })
		);
	auto drift = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ -0.2, 1. }),
		std::vector<double>({ 0., -2. })
		);
	RNGImpl rng(3489723983);
	BrownianMotion bm(std::move(vol_sqr), std::move(drift));
	ASSERT_EQ(nullptr, vol_sqr);
	ASSERT_EQ(nullptr, drift);
	ASSERT_EQ(0., bm.get_first_time());
	ASSERT_EQ(1., bm.get_last_time());
}

TEST(BrownianMotion, get_next_state) {
	auto vol_sqr = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ 0, 1.2 }),
		std::vector<double>({ 0., 120. })
		);
	auto drift = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ -0.2, 1. }),
		std::vector<double>({ 0., -2.4 })
		);
	RNGImpl rng(42);
	BrownianMotion bm(std::move(vol_sqr), std::move(drift));
	/*for (size_t i = 0; i < 4; ++i) {
		std::cout << std::setprecision(16) <<  rng->draw_gaussian() << std::endl;
	}*/
	// Liczby losowe kt�re wylosujemy:
	const std::array<double, 4> epsilons = {
		1.293820423272937,
		0.7049882664208599,
		0.3979773961837889,
		-0.5740948067202614
	};
	ASSERT_NEAR(-2. + epsilons[0] * 10., bm.get_next_state(rng, 0., 0., 1.), 1e-14);
	ASSERT_NEAR(1.5 - 1. + epsilons[1] * 10. / std::sqrt(2.), bm.get_next_state(rng, 1.5, 0.25, 0.75), 1e-14);
	ASSERT_THROW(bm.get_next_state(rng, 0.1, -0.1, 0.5), std::domain_error);
	ASSERT_THROW(bm.get_next_state(rng, 0.1, 0.1, 5), std::domain_error);
}

TEST(BrownianMotion, instant_switch) {
	auto vol_sqr = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ 0, 1.2 }),
		std::vector<double>({ 0., 120. })
		);
	auto drift = std::make_unique<const LinearInterpolator1D<seconds_t, double>>(
		std::vector<seconds_t>({ -0.2, 1. }),
		std::vector<double>({ 0., -2.4 })
		);
	RNGImpl rng(42);
	BrownianMotion bm(std::move(vol_sqr), std::move(drift));
	ASSERT_EQ(0.345, bm.instant_switch(rng, 0.345, 0.65));
	ASSERT_THROW(bm.instant_switch(rng, 0.1, -0.1), std::domain_error);
	ASSERT_THROW(bm.instant_switch(rng, 0.1, 100), std::domain_error);
}
