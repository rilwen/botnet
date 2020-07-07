#include <gtest/gtest.h>
#include "simbo/activity_state.hpp"
#include "simbo/enums_json.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(ActivityState, from_string) {
	ActivityState as;
	from_string("OFF", as);
	ASSERT_EQ(ActivityState::OFF, as);
	from_string("ON", as);
	ASSERT_EQ(ActivityState::ON, as);
	ASSERT_THROW(from_string("boo", as), std::invalid_argument);
}

TEST(ActivityState, to_string) {
	std::string str = to_string(ActivityState::OFF);
	ASSERT_EQ("OFF", str);
	str = to_string(ActivityState::ON);
	ASSERT_EQ("ON", str);
}

TEST(ActivityState, from_json) {
	json j = "\"OFF\""_json;
	ActivityState as;
	from_json(j, as);
	ASSERT_EQ(ActivityState::OFF, as);
}

TEST(ActivityState, to_json) {
	json j = ActivityState::ON;
	ASSERT_TRUE(j.is_string());
	ASSERT_EQ("\"ON\"", j.dump());
}
