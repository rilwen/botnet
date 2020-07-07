#include <fstream>
#include <gtest/gtest.h>
#include <boost/format.hpp>
#include "simbo/botnet_config.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(BotnetConfig, from_json) {	
	std::ifstream json_file("test_simbo/resources/test_botnet_config_from_json.json");
	json j;
	json_file >> j;
	BotnetConfig config = j;
	const DateTime date_time(Date(2018, 1, 1), TimeDuration(8, 0, 0, 0));
	ASSERT_EQ(0.1, config.probability_of_reconfiguration.get_ys()[0]);
	ASSERT_EQ(2, config.bot_state_weights.size());
	ASSERT_NEAR(0.2, config.bot_state_weights(date_time)->get_probability(0), 1e-15);
	ASSERT_NEAR(0.5, config.bot_state_weights(date_time)->get_probability(1), 1e-15);
	ASSERT_NEAR(0.3, config.bot_state_weights(date_time)->get_probability(2), 1e-15);
	ASSERT_EQ(std::vector<DateTime>({ DateTime(Date(2018, 8, 1), TimeDuration()), DateTime(Date(2018, 8, 2), TimeDuration()) }), config.infectious_email_sent_per_second_by_command_centre.get_xs());
	ASSERT_EQ(std::vector<double>({ 100, 0 }), config.infectious_email_sent_per_second_by_command_centre.get_ys());
	ASSERT_EQ(2000, config.email_address_list_size);
	ASSERT_EQ(2, config.payload.size());
	ASSERT_EQ(std::vector<int>({ 1, 2 }), config.payload.get_xs());
}