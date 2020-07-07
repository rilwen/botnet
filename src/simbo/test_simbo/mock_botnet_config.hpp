#pragma once
#include "simbo/botnet_config.hpp"

namespace simbo {
	BotnetConfig make_mock_botnet_config(seconds_t setting_up_period, seconds_t ping_interval,
		seconds_t reconfiguration_interval, bool reconfigure_at_start, bool ping_at_start, double probability_of_reconfiguration,
		double infectious_emails_sent_per_second_per_bot);
}
