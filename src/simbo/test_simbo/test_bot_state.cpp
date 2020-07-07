#include <gtest/gtest.h>
#include "simbo/bot_state.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(BotState, int_values) {
	ASSERT_EQ(0, static_cast<int>(BotState::SETTING_UP));
	ASSERT_LT(BotState::DORMANT, BotState::NONE);
	ASSERT_LT(BotState::WORKING, BotState::NONE);
	ASSERT_LT(BotState::PROPAGATING, BotState::NONE);
}

TEST(BotState, to_string) {
	ASSERT_STREQ("SETTING_UP", to_string(BotState::SETTING_UP));
	ASSERT_STREQ("WORKING", to_string(BotState::WORKING));
	ASSERT_STREQ("PROPAGATING", to_string(BotState::PROPAGATING));
	ASSERT_STREQ("DORMANT", to_string(BotState::DORMANT));
	ASSERT_STREQ("NONE", to_string(BotState::NONE));
}

TEST(BotState, from_string) {
	BotState state;
	from_string("SETTING_UP", state);
	ASSERT_EQ(BotState::SETTING_UP, state);
	from_string("WORKING", state);
	ASSERT_EQ(BotState::WORKING, state);
	from_string("PROPAGATING", state);
	ASSERT_EQ(BotState::PROPAGATING, state);
	from_string("DORMANT", state);
	ASSERT_EQ(BotState::DORMANT, state);
	from_string("NONE", state);
	ASSERT_EQ(BotState::NONE, state);
}

TEST(BotState, to_json) {
	json j;
	j = BotState::DORMANT;
	ASSERT_EQ("\"DORMANT\"", j.dump());
	j = BotState::NONE;
	ASSERT_EQ("null", j.dump());
}

TEST(BotState, from_json) {
	json j = "\"WORKING\""_json;
	BotState state = j;
	ASSERT_EQ(BotState::WORKING, state);
	j = nullptr;
	state = j;
	ASSERT_EQ(BotState::NONE, state);
}