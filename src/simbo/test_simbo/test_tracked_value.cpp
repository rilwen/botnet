#include <gtest/gtest.h>
#include "simbo/tracked_value.hpp"

using namespace simbo;

typedef TrackedValue<int> TestedClass;

TEST(TrackedValue, constructor) {
	TestedClass tv(-1, 0.4);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_last_change_time());
	ASSERT_EQ(0.4, tv.get_last_update_time());
}

TEST(TrackedValue, constructor_throws) {
	ASSERT_THROW(TestedClass(0.1, undefined_time()), std::domain_error);
}

TEST(TrackedValue, operator_equal) {
	TestedClass tv1(-1, 0.4);
	TestedClass tv2(-1, 0.4);
	TestedClass tv3(-1, 0.5);
	TestedClass tv4(2, 0.4);
	TestedClass tv5(2, 0.5);
	ASSERT_TRUE(tv1 == tv1);
	ASSERT_TRUE(tv1 == tv2);
	ASSERT_TRUE(tv2 == tv1);
	ASSERT_FALSE(tv1 == tv3);
	ASSERT_FALSE(tv3 == tv1);
	ASSERT_FALSE(tv1 == tv4);
	ASSERT_FALSE(tv4 == tv1);
	ASSERT_FALSE(tv1 == tv5);
	ASSERT_FALSE(tv5 == tv1);
}

TEST(TrackedValue, operator_not_equal) {
	TestedClass tv1(-1, 0.4);
	TestedClass tv2(-1, 0.4);
	TestedClass tv3(-1, 0.5);
	TestedClass tv4(2, 0.4);
	TestedClass tv5(2, 0.5);
	ASSERT_FALSE(tv1 != tv1);
	ASSERT_FALSE(tv1 != tv2);
	ASSERT_FALSE(tv2 != tv1);
	ASSERT_TRUE(tv1 != tv3);
	ASSERT_TRUE(tv3 != tv1);
	ASSERT_TRUE(tv1 != tv4);
	ASSERT_TRUE(tv4 != tv1);
	ASSERT_TRUE(tv1 != tv5);
	ASSERT_TRUE(tv5 != tv1);
}

TEST(TrackedValue, default_constructor) {
	TestedClass tv;
	ASSERT_EQ(0, tv.get_value());
	ASSERT_TRUE(is_time_undefined(tv.get_last_change_time()));
	ASSERT_TRUE(is_time_undefined(tv.get_last_update_time()));
}

TEST(TrackedValue, update_throws) {
	TestedClass tv;
	ASSERT_THROW(tv.update(1, 0.2), std::logic_error);
	tv = TestedClass(1, 0.5);
	ASSERT_THROW(tv.update(2, undefined_time()), std::invalid_argument);
	ASSERT_THROW(tv.update(2, 0.4), std::domain_error);
	ASSERT_THROW(tv.update(2, 0.5), std::domain_error);
}

TEST(TrackedValue, update) {
	TestedClass tv(-1, 0.4);
	tv.update(-1, 0.6);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_last_change_time());
	ASSERT_EQ(0.6, tv.get_last_update_time());
	tv.update(2, 0.7);
	ASSERT_EQ(2, tv.get_value());
	ASSERT_EQ(0.7, tv.get_last_change_time());
	ASSERT_EQ(0.7, tv.get_last_update_time());
}
