#include <gtest/gtest.h>
#include "simbo/timestamped_value.hpp"

using namespace simbo;

typedef TimestampedValue<int> TestedClass;

TEST(TimestampedValue, constructor) {
	TestedClass tv(-1, 0.4);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_time());
}

TEST(TimestampedValue, default_constructor) {
	TestedClass tv;
	ASSERT_EQ(0, tv.get_value());
	ASSERT_TRUE(is_time_undefined(tv.get_time()));
}

TEST(TimestampedValue, update_if_different_throws) {
	TestedClass tv;
	ASSERT_THROW(tv.update_if_different(1, 0.2), std::logic_error);
	tv = TestedClass(1, 0.5);
	ASSERT_THROW(tv.update_if_different(2, undefined_time()), std::invalid_argument);
	ASSERT_THROW(tv.update_if_different(2, 0.4), std::domain_error);
	ASSERT_THROW(tv.update_if_different(2, 0.5), std::domain_error);
}

TEST(TimestampedValue, update_if_different) {
	TestedClass tv(-1, 0.4);
	tv.update_if_different(-1, 0.6);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_time());
	tv.update_if_different(2, 0.5);
	ASSERT_EQ(2, tv.get_value());
	ASSERT_EQ(0.5, tv.get_time());
}

TEST(TimestampedValue, update_throws) {
	TestedClass tv;
	ASSERT_THROW(tv.update(1, 0.2), std::logic_error);
	tv = TestedClass(1, 0.5);
	ASSERT_THROW(tv.update(2, undefined_time()), std::invalid_argument);
	ASSERT_THROW(tv.update(2, 0.4), std::domain_error);
	ASSERT_THROW(tv.update(2, 0.5), std::domain_error);
}

TEST(TimestampedValue, update) {
	TestedClass tv(-1, 0.4);
	tv.update(-1, 0.6);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.6, tv.get_time());
	tv.update(2, 0.7);
	ASSERT_EQ(2, tv.get_value());
	ASSERT_EQ(0.7, tv.get_time());
}

TEST(TimestampedValue, reset_value) {
	TestedClass tv(-1, 0.4);
	tv.reset_value(1);
	ASSERT_EQ(1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_time());
}

TEST(TimestampedValue, make_timestamped_value) {
	const TimestampedValue<int> tv = make_timestamped_value(-1, 0.4);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_time());
}

TEST(TimestampedValue, make_timestamped_const_ptr) {
	double x;
	const TimestampedValue<double *> tv1(&x, 0.2);
	const TimestampedValue<const double*> tv2 = make_timestamped_const_ptr(tv1);
	ASSERT_EQ(tv1.get_value(), tv2.get_value());
	ASSERT_EQ(tv1.get_time(), tv2.get_time());
}
