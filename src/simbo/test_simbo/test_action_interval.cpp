#include <gtest/gtest.h>
#include "simbo/action_interval.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(ActionInterval, from_json) {
	json j = "{\"period\":300.5,\"by_wall_clock\":true}"_json;
	ActionInterval ai = j;
	ASSERT_EQ(300.5, ai.get_period());
	ASSERT_TRUE(ai.is_by_wall_clock());
}

//TEST(ActionInterval, to_json) {
//	ActionInterval ai(120, false);
//	json j = ai;
//	ASSERT_EQ("{\"by_wall_clock\":false,\"period\":120.0}", j.dump());
//}
