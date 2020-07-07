#include "simbo/sparse_discrete_random_variable.hpp"
#include "mock_botnet_config.hpp"

namespace simbo {
	simbo::BotnetConfig make_mock_botnet_config(seconds_t setting_up_period, seconds_t ping_interval,
		seconds_t reconfiguration_interval, bool reconfigure_at_start, bool ping_at_start, double probability_of_reconfiguration,
		double infectious_emails_sent_per_second_per_bot) {
		BotnetPayloadConfig payload_config;
		payload_config.ping_interval = ActionInterval(ping_interval, true);
		payload_config.reconfiguration_interval = ActionInterval(reconfiguration_interval, true);
		payload_config.ping_at_start = ping_at_start;
		payload_config.reconfigure_at_start = reconfigure_at_start;
		payload_config.infectious_emails_sent_per_second_per_bot = infectious_emails_sent_per_second_per_bot;

		BotnetConfig botnet_config;

		botnet_config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_config);

		botnet_config.dropper.setting_up_period = setting_up_period;

		botnet_config.probability_of_reconfiguration = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), probability_of_reconfiguration);
		botnet_config.bot_state_weights = PiecewiseConstantInterpolator1D<DateTime, std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>(
			std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()) }),
			std::vector<std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>({
			std::make_shared<SparseDiscreteRandomVariable<BotState>>(std::vector<double>({ 0.4, 0.1, 0.5 }), std::vector<BotState>({ BotState::WORKING, BotState::PROPAGATING, BotState::DORMANT }))
		}));
		botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 0);
		botnet_config.email_address_list_size = 0;
		botnet_config.payload_release_schedule = PiecewiseConstantInterpolator1D<DateTime, int>(min_datetime(), max_datetime(), 1);
		botnet_config.validate();
		return botnet_config;
	}
}
