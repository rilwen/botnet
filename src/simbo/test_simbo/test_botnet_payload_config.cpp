#include <gtest/gtest.h>
#include "simbo/botnet_payload_config.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(BotnetPayloadConfig, from_json) {
	json j = "{\"infectious_emails_sent_per_second_per_bot\":0.05,\"ping_interval\":{\"by_wall_clock\":false,\"period\":300.0},\"ping_at_start\":true,\"reconfiguration_interval\":{\"by_wall_clock\":true,\"period\":3600.0},\"reconfigure_at_start\":false,\"min_detecting_av_release\":{\"AV\":5}}"_json;
	BotnetPayloadConfig config = j;
	const DateTime date_time(Date(2018, 1, 1), TimeDuration(8, 0, 0, 0));
	ASSERT_FALSE(config.reconfigure_at_start);
	ASSERT_TRUE(config.ping_at_start);
	ASSERT_EQ(0.05, config.infectious_emails_sent_per_second_per_bot);
	ASSERT_EQ(1, config.min_detecting_av_release.size());
	ASSERT_EQ(5, config.min_detecting_av_release["AV"]);
}