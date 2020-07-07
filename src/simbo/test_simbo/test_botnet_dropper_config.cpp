#include <gtest/gtest.h>
#include "simbo/botnet_dropper_config.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(BotnetDropperConfig, from_json) {
	json j = "{\"max_infected_release\":[[[\"WINDOWS\",7], 2],[[\"LINUX\",24], 1]],\"setting_up_period\":60.0}"_json;
	BotnetDropperConfig config = j;
	const DateTime date_time(Date(2018, 1, 1), TimeDuration(8, 0, 0, 0));
	ASSERT_EQ(2, config.max_infected_release.size());
	ASSERT_EQ(2, config.max_infected_release[std::make_pair(OperatingSystemType::WINDOWS, 7)]);
	ASSERT_EQ(1, config.max_infected_release[std::make_pair(OperatingSystemType::LINUX, 24)]);
	ASSERT_EQ(60, config.setting_up_period);	
}