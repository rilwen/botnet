#include <gtest/gtest.h>
#include "simbo/tracked_value_own_time.hpp"

using namespace simbo;

typedef TrackedValueOwnTime<int> TestedClass;

TEST(TrackedValueOwnTime, constructor) {
	TestedClass tv(-1, 0.4);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.4, tv.get_last_change_time());
	ASSERT_EQ(0.4, tv.get_last_update_time());
	ASSERT_EQ(0., tv.get_own_time_since_last_change());
	ASSERT_EQ(0., tv.get_own_time_since_last_update());
}

TEST(TrackedValueOwnTime, constructor_throws) {
	ASSERT_THROW(TestedClass(0.1, undefined_time()), std::domain_error);
}

TEST(TrackedValueOwnTime, update) {
	TestedClass tv(-1, 0.4);
	tv.update(-1, 0.5, true);
	ASSERT_EQ(-1, tv.get_value());
	ASSERT_EQ(0.5, tv.get_last_update_time());
	ASSERT_EQ(0., tv.get_own_time_since_last_update());
	ASSERT_NEAR(0.1, tv.get_own_time_since_last_change(), 1e-14);
	tv.update(1, 0.6, true);
	ASSERT_EQ(1, tv.get_value());
	ASSERT_EQ(0.6, tv.get_last_change_time());
	ASSERT_EQ(0.6, tv.get_last_update_time());
	ASSERT_EQ(0., tv.get_own_time_since_last_update());
	ASSERT_EQ(0., tv.get_own_time_since_last_change());
	tv.update(1, 0.75, true);
	ASSERT_EQ(0., tv.get_own_time_since_last_update());
	ASSERT_NEAR(0.15, tv.get_own_time_since_last_change(), 1e-14);
	tv.update(1, 0.9, false);
	ASSERT_EQ(0., tv.get_own_time_since_last_update());
	ASSERT_NEAR(0.15, tv.get_own_time_since_last_change(), 1e-14);
}

TEST(TrackedValueOwnTime, update_own_time) {
	TestedClass tv(-1, 0.4);
	tv.update_own_time(0.5, false);
	ASSERT_NEAR(0., tv.get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(0., tv.get_own_time_since_last_update(), 1e-14);
	tv.update(-1, 0.6, true);
	ASSERT_NEAR(0.1, tv.get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(0.0, tv.get_own_time_since_last_update(), 1e-14);
	tv.update_own_time(0.7, false);
	ASSERT_NEAR(0.1, tv.get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(0.0, tv.get_own_time_since_last_update(), 1e-14);
	tv.update_own_time(0.81, true);
	ASSERT_NEAR(0.21, tv.get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(0.11, tv.get_own_time_since_last_update(), 1e-14);
}
