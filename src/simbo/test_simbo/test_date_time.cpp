#include <cmath>
#include <limits>
#include <gtest/gtest.h>
#include "simbo/date_time.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(DateTime, to_seconds) {
	TimeDuration duration(0, 0, 30, 5);
	// Część ułamkowa TimeDuration ma być w mikrosekundach.
	ASSERT_NEAR(30 + 5e-6, to_seconds(duration), 1e-12);
	duration = TimeDuration(0, 0, -30, -5000);
	ASSERT_NEAR(-30 - 5e-3, to_seconds(duration), 1e-12);
	duration = TimeDuration(2, 12, 26, 199);
	ASSERT_NEAR(2 * 3600 + 12 * 60 + 26 + 199e-6, to_seconds(duration), 1e-12);
	ASSERT_EQ(0, to_seconds(TimeDuration()));
}

TEST(DateTime, time_duration_from_seconds) {
	ASSERT_EQ(TimeDuration(0, 1, 1, 0), time_duration_from_seconds(61));
	ASSERT_EQ(-TimeDuration(0, 1, 1, 0), time_duration_from_seconds(-61));
	ASSERT_EQ(TimeDuration(0, 1, 1, 0.5), time_duration_from_seconds(61.5));
	ASSERT_EQ(TimeDuration(1, 1, 1, 0.15), time_duration_from_seconds(3661.15));
}

TEST(DateTime, date_from_string) {
	const Date expected(2016, 6, 23);
	const Date actual = from_string("2016-06-23");
	ASSERT_EQ(expected, actual);
}

TEST(DateTime, date_to_string) {
	const std::string expected("2016-06-23");
	const std::string actual = to_string(Date(2016, 6, 23));
	ASSERT_EQ(expected, actual);
}

TEST(DateTime, infinity) {
	ASSERT_EQ(std::numeric_limits<double>::infinity(), infinite_time());
}

TEST(DateTime, undefined) {
	ASSERT_TRUE(std::isnan(undefined_time()));
}

TEST(DateTime, is_time_undefined) {
	ASSERT_TRUE(is_time_undefined(undefined_time()));
	ASSERT_FALSE(is_time_undefined(infinite_time()));
	ASSERT_FALSE(is_time_undefined(0.1));
}

TEST(DateTime, is_time_finite) {
	ASSERT_FALSE(is_time_finite(undefined_time()));
	ASSERT_FALSE(is_time_finite(-infinite_time()));
	ASSERT_FALSE(is_time_finite(infinite_time()));
	ASSERT_TRUE(is_time_finite(0.1));
}

TEST(DateTime, date_to_json) {
	Date date(1989, 6, 4);
	json j = date;
	ASSERT_EQ("\"1989-06-04\"", j.dump());
}

TEST(DateTime, date_from_json) {
	json j = "{\"day\":4,\"month\":6,\"year\":1989}"_json;
	Date date = j;
	ASSERT_EQ(Date(1989, 6, 4), date);
}

TEST(DateTime, date_from_json_string) {
	json j = "\"1989-06-04\""_json;
	Date date = j;
	ASSERT_EQ(Date(1989, 6, 4), date);
}

TEST(DateTime, time_duration_to_json) {
	TimeDuration td(2, 31, 3, 1);
	json j = td;
	ASSERT_EQ("{\"hours\":2,\"microseconds\":1,\"minutes\":31,\"seconds\":3}", j.dump());
	td = TimeDuration(0, 20, 0, 0);
	j = td;
	ASSERT_EQ("{\"hours\":0,\"microseconds\":0,\"minutes\":20,\"seconds\":0}", j.dump());
}

TEST(DateTime, time_duration_from_json) {
	json j = "{\"hours\":2,\"minutes\":31,\"seconds\":3}"_json;
	TimeDuration td = j;
	ASSERT_EQ(TimeDuration(2, 31, 3, 0), td);
}

TEST(DateTime, datetime_to_json) {
	DateTime high_noon(Date(1989, 6, 4), TimeDuration(12, 0, 0, 0));
	json j = high_noon;
	ASSERT_EQ("{\"date\":\"1989-06-04\",\"time\":{\"hours\":12,\"microseconds\":0,\"minutes\":0,\"seconds\":0}}", j.dump());
}

TEST(DateTime, datetime_from_json) {
	DateTime expected(Date(1989, 6, 4), TimeDuration(12, 0, 0, 0));
	json j = "{\"date\":{\"day\":4,\"month\":6,\"year\":1989},\"time\":{\"hours\":12,\"microseconds\":0,\"minutes\":0,\"seconds\":0}}"_json;
	DateTime actual = j;
	ASSERT_EQ(expected, actual);
}

TEST(DateTime, datetime_from_json_string) {
	DateTime expected(Date(1989, 6, 4), TimeDuration(0, 0, 0, 0));
	json j = "\"1989-06-04\""_json;
	DateTime actual = j;
	ASSERT_EQ(expected, actual);
}

TEST(DateTime, is_weekend) {
	ASSERT_FALSE(is_weekend(Date(2018, 8, 1)));
	ASSERT_FALSE(is_weekend(Date(2018, 8, 2)));
	ASSERT_FALSE(is_weekend(Date(2018, 8, 3)));
	ASSERT_TRUE(is_weekend(Date(2018, 8, 4)));
	ASSERT_TRUE(is_weekend(Date(2018, 8, 5)));
	ASSERT_FALSE(is_weekend(Date(2018, 8, 6)));
	ASSERT_FALSE(is_weekend(Date(2018, 8, 7)));
}
