#include <gtest/gtest.h>
#include "simbo/semi_predetermined_process.hpp"
#include "mock_rng.hpp"

using namespace simbo;

typedef SemiPredeterminedProcess<int> TestedClass;

TEST(SemiPredeterminedProcess, constructor) {
	const TestedClass process(std::vector<seconds_t>(1, 0.5), std::vector<seconds_t>(1, 1.5), std::vector<int>(1, 2));
	ASSERT_EQ(0.5, process.get_first_time());
	ASSERT_EQ(1.5, process.get_last_time());
}

TEST(SemiPredeterminedProcess, constructor_throws) {
	ASSERT_THROW(TestedClass(std::vector<seconds_t>(), std::vector<seconds_t>(1, 1.5), std::vector<int>(1, 2)), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<seconds_t>(1, 0.5), std::vector<seconds_t>(), std::vector<int>(1, 2)), std::invalid_argument);
	ASSERT_THROW(TestedClass(std::vector<seconds_t>(1, 0.5), std::vector<seconds_t>(1, 1.5), std::vector<int>()), std::invalid_argument);
}

TEST(SemiPredeterminedProcess, instant_switch_throws) {
	MockRNG rng;
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<seconds_t>({ 1.5, 2.5 }), std::vector<int>({ 1, 2 }));
	ASSERT_THROW(process.instant_switch(rng, -1, 0.1), std::domain_error);
	ASSERT_THROW(process.instant_switch(rng, -1, 2.6), std::domain_error);
}

TEST(SemiPredeterminedProcess, instant_switch_deterministic) {
	MockRNG rng;
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<seconds_t>({ 1.5, 2.5 }), std::vector<int>({ 1, 2 }));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 0.5));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 0.75));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.5));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.75));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 2.5));
}

TEST(SemiPredeterminedProcess, instant_switch_semideterministic) {
	MockRNG rng;
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<seconds_t>({ 1., 2.5 }), std::vector<int>({ 1, 2 }));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 0.5));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 0.75));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.5));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.75));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 2.5));
	rng.add_uniform(0.1);
	rng.add_uniform(0.8);
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.25));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 1.25));
}

TEST(SemiPredeterminedProcess, instant_switch_fullyrandom) {
	MockRNG rng;
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<int>({ 1, 2 }));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 0.5));
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.5));
	rng.add_uniform(0.1);
	rng.add_uniform(0.8);
	ASSERT_EQ(2, process.instant_switch(rng, -1, 1.));
	ASSERT_EQ(1, process.instant_switch(rng, -1, 1.));
}

TEST(SemiPredeterminedProcess, get_next_state_throws) {
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5 }), std::vector<seconds_t>({ 1.5, 2.5 }), std::vector<int>({ 1, 2 }));
	MockRNG rng;
	ASSERT_THROW(process.get_next_state(rng, 1, 0.1, 1.), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 2, 2.1, 3.), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 2, 0.5, 0.6), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 1, 0.55, 0.55), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 1, 0.6, 0.55), std::domain_error);
	ASSERT_THROW(process.get_next_state(rng, 1, 1.6, 2.1), std::domain_error);
}

TEST(SemiPredeterminedProcess, get_next_state_deterministic) {
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5, 2.5 }), std::vector<seconds_t>({ 1.5, 2.5, 3.5 }), std::vector<int>({ 1, 2, 3 }));
	MockRNG rng;
	ASSERT_EQ(1, process.get_next_state(rng, 1, 0.55, 0.65));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 2));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 1.55, 2.49));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 1.55, 2.5));
	ASSERT_EQ(3, process.get_next_state(rng, 1, 0.5, 2.8));
}

TEST(SemiPredeterminedProcess, get_next_state_semideterministic) {
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5, 2.5 }), std::vector<seconds_t>({ 1., 2., 3. }), std::vector<int>({ 1, 2, 3 }));
	MockRNG rng;
	ASSERT_EQ(1, process.get_next_state(rng, 1, 0.55, 0.65));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 1.9));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 1.55, 1.99));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 1.55, 2.5));
	ASSERT_EQ(3, process.get_next_state(rng, 1, 0.5, 2.8));
	// Przejście już zaszło.
	ASSERT_EQ(2, process.get_next_state(rng, 2, 1.25, 1.375));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 1.25, 1.375));
	ASSERT_EQ(3, process.get_next_state(rng, 3, 2.25, 2.375));
	ASSERT_EQ(3, process.get_next_state(rng, 3, 2.25, 2.375));
	rng.add_uniform(0.1);
	rng.add_uniform(0.8);
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 1.25));
	ASSERT_EQ(1, process.get_next_state(rng, 1, 0.55, 1.25));
	ASSERT_EQ(3, process.get_next_state(rng, 1, 0.55, 2.25));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 2.25));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 1.25, 1.375));
	ASSERT_EQ(1, process.get_next_state(rng, 1, 1.25, 1.375));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 2.25, 2.375));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 2.25, 2.375));
}

TEST(SemiPredeterminedProcess, get_next_state_fullyrandom) {
	MockRNG rng;
	const TestedClass process(std::vector<seconds_t>({ 0.5, 1.5, 2.5 }), std::vector<seconds_t>({ 0.5, 1.5, 2.5 }), std::vector<int>({ 1, 2, 3 }));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 1.5));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 0.55, 1.5));
	ASSERT_EQ(3, process.get_next_state(rng, 1, 0.55, 2.5));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 0.55, 2.5));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 1.55, 2.5));
	ASSERT_EQ(3, process.get_next_state(rng, 3, 1.55, 2.5));
	// Przejście już zaszło.
	ASSERT_EQ(2, process.get_next_state(rng, 2, 0.55, 0.75));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 0.55, 0.75));
	ASSERT_EQ(3, process.get_next_state(rng, 3, 1.75, 2.25));
	ASSERT_EQ(3, process.get_next_state(rng, 3, 1.75, 2.25));
	rng.add_uniform(0.1);
	rng.add_uniform(0.8);
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 0.75));
	ASSERT_EQ(1, process.get_next_state(rng, 1, 0.55, 0.75));
	ASSERT_EQ(3, process.get_next_state(rng, 1, 0.55, 1.75));
	ASSERT_EQ(2, process.get_next_state(rng, 1, 0.55, 1.75));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 0.55, 1.75));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 0.55, 1.75));
	ASSERT_EQ(3, process.get_next_state(rng, 2, 1.75, 2));
	ASSERT_EQ(2, process.get_next_state(rng, 2, 1.75, 2));
}

TEST(SemiPredeterminedProcess, make_seasonal_weekly) {
	std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_periods_workdays({
		std::make_pair(TimeDuration(8, 0, 0, 0), TimeDuration(17, 0, 0, 0)),
		std::make_pair(TimeDuration(20, 30, 0, 0), TimeDuration(23, 30, 0, 0))
	});
	std::vector<int> deterministic_values_workdays({ 0, 1 });
	std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_periods_weekend({
		std::make_pair(TimeDuration(10, 0, 0, 0), TimeDuration(22, 0, 0, 0))
	});
	std::vector<int> deterministic_values_weekend({ 1 });
	const DateTime reference_date_time(Date(2018, 8, 1), TimeDuration(0, 0, 0, 0));
	const Date start_date(2018, 8, 3);
	const Date end_date(2018, 8, 4);
	const auto process = TestedClass::make_seasonal_weekly(deterministic_periods_workdays, deterministic_values_workdays, deterministic_periods_weekend, deterministic_values_weekend, reference_date_time, start_date, end_date);
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(to_seconds(DateTime(start_date, deterministic_periods_workdays[0].first) - reference_date_time), process->get_first_time());
	ASSERT_EQ(to_seconds(DateTime(end_date, deterministic_periods_weekend[0].second) - reference_date_time), process->get_last_time());
	ASSERT_EQ(std::vector<int>({ 0, 1, 1 }), process->get_values());
	const seconds_t offset = 48 * 3600;
	ASSERT_EQ(std::vector<seconds_t>({offset + 8 * 3600, offset + 20.5 * 3600, offset + 34 * 3600}), process->get_lower_bounds());
	ASSERT_EQ(std::vector<seconds_t>({ offset + 17 * 3600, offset + 23.5 * 3600, offset + 46 * 3600 }), process->get_upper_bounds());
}

TEST(SemiPredeterminedProcess, from_json_seasonal_weekly) {
	const json j = "{\"deterministic_periods_workdays\":[[{\"hours\":8},{\"hours\":17}],[{\"hours\":20,\"minutes\":30},{\"hours\":23,\"minutes\":30}]],\"deterministic_values_workdays\":[0,1],\"deterministic_periods_weekend\":[[{\"hours\":10},{\"hours\":22}]],\"deterministic_values_weekend\":[1]}"_json;
	std::unique_ptr<TestedClass> process;
	const Date schedule_start_date(2018, 8, 3);
	const int nbr_points = 2;
	const DateTime reference_date_time(schedule_start_date, TimeDuration(0, 0, 0, 0));
	const Schedule schedule(reference_date_time, TimeDuration(24, 0, 0, 0), nbr_points);
	from_json_seasonal_weekly(j, schedule, process);
	const Date process_start_date = schedule_start_date - boost::gregorian::days(1);
	const Date process_end_date = schedule_start_date + boost::gregorian::days(nbr_points);
	std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_periods_workdays({
		std::make_pair(TimeDuration(8, 0, 0, 0), TimeDuration(17, 0, 0, 0)),
		std::make_pair(TimeDuration(20, 30, 0, 0), TimeDuration(23, 30, 0, 0))
	});
	std::vector<int> deterministic_values_workdays({ 0, 1 });
	std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_periods_weekend({
		std::make_pair(TimeDuration(10, 0, 0, 0), TimeDuration(22, 0, 0, 0))
	});
	std::vector<int> deterministic_values_weekend({ 1 });
	ASSERT_NE(nullptr, process);
	ASSERT_EQ(to_seconds(DateTime(process_start_date, deterministic_periods_workdays[0].first) - reference_date_time), process->get_first_time());
	ASSERT_EQ(to_seconds(DateTime(process_end_date, deterministic_periods_weekend[0].second) - reference_date_time), process->get_last_time());
	ASSERT_EQ(std::vector<int>({ 0, 1, 0, 1, 1, 1 }), process->get_values());
}
