#include <gtest/gtest.h>
#include "simbo/json.hpp"
#include "simbo/schedule.hpp"
#include "simbo/utils.hpp"

using namespace simbo;

TEST(Schedule, constructor_one_point) {
	const DateTime start(Date(2016, 4, 20), TimeDuration(18, 30, 0, 0));
	const Schedule schedule(start);
	ASSERT_EQ(1, schedule.size());
	ASSERT_EQ(start, schedule.get_point(0));
	ASSERT_EQ(start, schedule.get_start());
	ASSERT_EQ(start, schedule.get_end());
}

TEST(Schedule, constructor_fixed_interval) {
	const Date start_date(1999, 12, 31);
	const DateTime start(start_date, TimeDuration(0, 0, 0, 0));
	const TimeDuration step_size(0, 30, 0, 0);
	const seconds_t step_size_sec = to_seconds(step_size);
	const int nbr_points = 49;
	const Schedule schedule(start, step_size, nbr_points);
	ASSERT_EQ(nbr_points, schedule.size());
	for (int i = 0; i < nbr_points / 2; ++i) {
		ASSERT_EQ(DateTime(start_date, TimeDuration(static_cast<boost::posix_time::time_duration::hour_type>(i), 0, 0, 0)), schedule.get_point(2 * i)) << i;
		ASSERT_EQ(DateTime(start_date, TimeDuration(static_cast<boost::posix_time::time_duration::hour_type>(i), 30, 0, 0)), schedule.get_point(2 * i + 1)) << i;
	}
	ASSERT_EQ(start, schedule.get_start());
	ASSERT_EQ(schedule.get_point(nbr_points - 1), schedule.get_end());
}

TEST(Schedule, constructor_list_of_intervals_from_start_empty_vec) {
	std::vector<seconds_t> intervals;
	Schedule schedule(std::move(intervals));
	ASSERT_EQ(1, schedule.size());
	ASSERT_EQ(0., schedule.get_interval_from_start(0));
	ASSERT_EQ(DateTime(Date(1970, 1, 1)), schedule.get_point(0));
}

TEST(Schedule, constructor_list_of_intervals_from_start_vec_with_zero) {
	std::vector<seconds_t> intervals({ 0. });
	Schedule schedule(std::move(intervals));
	ASSERT_EQ(1, schedule.size());
	ASSERT_EQ(0., schedule.get_interval_from_start(0));
	ASSERT_EQ(DateTime(Date(1970, 1, 1)), schedule.get_point(0));
}

TEST(Schedule, constructor_list_of_intervals_from_start_vec_with_nonzero) {
	std::vector<seconds_t> intervals({ 3600. });
	Schedule schedule(std::move(intervals));
	ASSERT_EQ(2, schedule.size());
	ASSERT_EQ(0., schedule.get_interval_from_start(0));
	ASSERT_EQ(3600., schedule.get_interval_from_start(1));
	ASSERT_EQ(DateTime(Date(1970, 1, 1)), schedule.get_point(0));
	ASSERT_EQ(DateTime(Date(1970, 1, 1), TimeDuration(1, 0, 0, 0)), schedule.get_point(1));
}

TEST(Schedule, constructor_list_of_intervals_from_start_vec_with_zero_and_nonzero) {
	std::vector<seconds_t> intervals({ 0., 3601.5 });
	Schedule schedule(std::move(intervals));
	ASSERT_EQ(2, schedule.size());
	ASSERT_EQ(0., schedule.get_interval_from_start(0));
	ASSERT_EQ(3601.5, schedule.get_interval_from_start(1));
	ASSERT_EQ(DateTime(Date(1970, 1, 1)), schedule.get_point(0));
	ASSERT_EQ(DateTime(Date(1970, 1, 1), TimeDuration(1, 0, 1, 0.5)), schedule.get_point(1));
}

TEST(Schedule, constructor_list_of_time_points) {
	std::vector<DateTime> time_points({DateTime(Date(2018, 8, 1), TimeDuration(0, 0, 0, 0)), DateTime(Date(2018, 8, 1), TimeDuration(12, 0, 0, 0))});
	Schedule schedule(make_copy(time_points));
	ASSERT_EQ(2, schedule.size());
	ASSERT_EQ(time_points[0], schedule.get_point(0));
	ASSERT_EQ(time_points[1], schedule.get_point(1));
	ASSERT_EQ(0., schedule.get_interval_from_start(0));
	ASSERT_EQ(to_seconds(time_points[1] - time_points[0]), schedule.get_interval_from_start(1));
}

TEST(Schedule, constructor_list_of_offsets_no_complete) {
	const Date start_date(2018, 8, 3);
	const int nbr_days = 2;
	const std::vector<TimeDuration> workday_offsets({ TimeDuration(0, 0, 0, 0), TimeDuration(12, 30, 0, 0), TimeDuration(23, 0, 0, 0) });
	const std::vector<TimeDuration> weekend_offsets({ TimeDuration(0, 0, 0, 0), TimeDuration(23, 30, 0, 0) });
	const Schedule schedule(start_date, workday_offsets, weekend_offsets, nbr_days, false);
	const DateTime ref_datetime(start_date, workday_offsets[0]);
	ASSERT_EQ(workday_offsets.size() + weekend_offsets.size(), schedule.size());
	for (size_t i = 0; i < workday_offsets.size(); ++i) {
		const auto int_i = static_cast<int>(i);
		ASSERT_EQ(DateTime(start_date, workday_offsets[i]), schedule.get_point(int_i)) << i;
		ASSERT_EQ(to_seconds(schedule.get_point(int_i) - ref_datetime), schedule.get_interval_from_start(int_i)) << i;
	}
	const Date next_day = start_date + boost::gregorian::days(1);
	for (size_t i = 0; i < weekend_offsets.size(); ++i) {
		const auto j = static_cast<int>(i + workday_offsets.size());
		ASSERT_EQ(DateTime(next_day, weekend_offsets[i]), schedule.get_point(j)) << i;
		ASSERT_EQ(to_seconds(schedule.get_point(j) - ref_datetime), schedule.get_interval_from_start(j)) << i;
	}
}

TEST(Schedule, constructor_list_of_offsets_complete) {
	const Date start_date(2018, 8, 3);
	const int nbr_days = 2;
	const std::vector<TimeDuration> workday_offsets({ TimeDuration(0, 0, 0, 0), TimeDuration(12, 30, 0, 0), TimeDuration(23, 0, 0, 0) });
	const std::vector<TimeDuration> weekend_offsets({ TimeDuration(0, 0, 0, 0), TimeDuration(23, 30, 0, 0) });
	const Schedule schedule(start_date, workday_offsets, weekend_offsets, nbr_days, true);
	const DateTime ref_datetime(start_date, workday_offsets[0]);
	ASSERT_EQ(workday_offsets.size() + weekend_offsets.size() + 1, schedule.size());
	for (size_t i = 0; i < workday_offsets.size(); ++i) {
		const auto int_i = static_cast<int>(i);
		ASSERT_EQ(DateTime(start_date, workday_offsets[i]), schedule.get_point(int_i)) << i;
		ASSERT_EQ(to_seconds(schedule.get_point(int_i) - ref_datetime), schedule.get_interval_from_start(int_i)) << i;
	}
	const Date next_day = start_date + boost::gregorian::days(1);
	for (size_t i = 0; i < weekend_offsets.size(); ++i) {
		const auto j = static_cast<int>(i + workday_offsets.size());
		ASSERT_EQ(DateTime(next_day, weekend_offsets[i]), schedule.get_point(j)) << i;
		ASSERT_EQ(to_seconds(schedule.get_point(j) - ref_datetime), schedule.get_interval_from_start(j)) << i;
	}
	ASSERT_EQ(DateTime(start_date + boost::gregorian::days(nbr_days), TimeDuration()), schedule.get_point(schedule.size() - 1));
}

TEST(Schedule, get_interval_seconds) {
	const Date start_date(1999, 12, 31);
	const DateTime start(start_date, TimeDuration(1, 23, 12, 100));
	const TimeDuration step_size(0, 27, 30, 0);
	const seconds_t step_size_sec = to_seconds(step_size);
	const int nbr_points = 12;
	const Schedule schedule(start, step_size, nbr_points);
	ASSERT_EQ(0., schedule.get_interval_seconds(3, 3));
	ASSERT_NEAR(step_size_sec, schedule.get_interval_seconds(5, 6), 1e-15);
	ASSERT_NEAR(-2 * step_size_sec, schedule.get_interval_seconds(4, 2), 1e-15);
	ASSERT_NEAR(3 * step_size_sec, schedule.get_interval_seconds(4, 7), 1e-15);
}

TEST(Schedule, get_seconds) {
	const Date start_date(1999, 12, 31);
	const DateTime start(start_date, TimeDuration(0, 0, 0, 0));
	const TimeDuration step_size(24, 0, 0, 0);
	const int nbr_points = 12;
	const Schedule schedule(start, step_size, nbr_points);
	const auto seconds = schedule.get_seconds();
	ASSERT_EQ(nbr_points, static_cast<int>(seconds.size()));
	for (int i = 0; i < nbr_points; ++i) {
		ASSERT_EQ(seconds[i], schedule.get_interval_from_start(i)) << i;
	}
}

TEST(Schedule, from_json) {
	json j = "{\"nbr_points\":10,\"period\":{\"hours\":1},\"start\":{\"date\":{\"day\":1,\"month\":2,\"year\":1990},\"time\":{}}}"_json;
	const Schedule expected(DateTime(Date(1990, 2, 1), TimeDuration(0, 0, 0, 0)), TimeDuration(1, 0, 0, 0), 10);
	const Schedule actual = j;
	ASSERT_EQ(expected.get_seconds(), actual.get_seconds());
}
