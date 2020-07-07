#include <gtest/gtest.h>
#include "simbo/linear_interpolator1d.hpp"
#include "simbo/predetermined_process.hpp"
#include "simbo/utils.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef PredeterminedProcess<double> TestedClass;

TEST(PredeterminedProcess, constructor_throws) {
	ASSERT_THROW(TestedClass(nullptr), std::invalid_argument);
	std::vector<seconds_t> times({ 0, 0, 0 });
	std::vector<double> values(3, 0);
	ASSERT_THROW(TestedClass{ TestedClass::values_interpolator_ptr(new LinearInterpolator1D<seconds_t, double>(std::move(times), std::move(values))) }, std::invalid_argument);
}

TEST(PredeterminedProcess, constructor) {
	std::vector<seconds_t> times({ -1, 1, 2.5 });
	std::vector<double> values(3, 0.);
	const TestedClass process(TestedClass::values_interpolator_ptr(new LinearInterpolator1D<seconds_t, double>(std::move(times), std::move(values))));
	ASSERT_EQ(-1, process.get_first_time());
	ASSERT_EQ(2.5, process.get_last_time());
	ASSERT_TRUE(times.empty());
	ASSERT_TRUE(values.empty());
}

TEST(PredeterminedProcess, get_next_state) {
	std::vector<seconds_t> times({ -1, 1, 2.5 });
	std::vector<double> values({ -0.2, 0.3, 0.1 });
	const TestedClass process(TestedClass::values_interpolator_ptr(new LinearInterpolator1D<seconds_t, double>(make_copy(times), make_copy(values))));
	const LinearInterpolator1D<seconds_t, double> interpolator(std::move(times), std::move(values));
	MockRNG rng;
	ASSERT_EQ(0.3, process.get_next_state(rng, -0.2, -1, 1));
	ASSERT_EQ(0.3, process.get_next_state(rng, interpolator(0), 0, 1));
	ASSERT_NEAR(0.2, process.get_next_state(rng, interpolator(0), 0, 1.75), 1e-12);
	ASSERT_THROW(process.get_next_state(rng, 0, 0, 1), std::domain_error);
}

TEST(PredeterminedProcess, instant_switch) {
	std::vector<seconds_t> times({ -1, 1, 2.5 });
	std::vector<double> values({ -0.2, 0.3, 0.1 });
	const TestedClass process(TestedClass::values_interpolator_ptr(new LinearInterpolator1D<seconds_t, double>(make_copy(times), make_copy(values))));
	const LinearInterpolator1D<seconds_t, double> interpolator(std::move(times), std::move(values));
	MockRNG rng;
	ASSERT_EQ(0.3, process.instant_switch(rng, -0.05, 1));
	ASSERT_THROW(process.instant_switch(rng, 0.05, -1.2), std::domain_error);
	ASSERT_THROW(process.instant_switch(rng, 0.05, 3.2), std::domain_error);
}
