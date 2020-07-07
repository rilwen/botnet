#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(Context, schedule) {
	const std::shared_ptr<RNG> rng(new MockRNG());
	const int nbr_time_points = 4;
	const int step_size_in_minutes = 2;
	const DateTime start_date(Date(1970, 1, 1), TimeDuration());
	Schedule schedule(start_date, TimeDuration(0, step_size_in_minutes, 0, 0), nbr_time_points);

	BotnetPayloadConfig payload_config;
	payload_config.reconfigure_at_start = false;
	payload_config.ping_interval = ActionInterval(480, false);
	payload_config.reconfiguration_interval = ActionInterval(720, true);
	payload_config.infectious_emails_sent_per_second_per_bot = 1e-3;

	BotnetConfig botnet_config;
	botnet_config.dropper.setting_up_period = 240;
	botnet_config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_config);
	botnet_config.probability_of_reconfiguration = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 0.9);
	
	botnet_config.bot_state_weights = PiecewiseConstantInterpolator1D<DateTime, std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>(
		std::vector<DateTime>({ start_date }),
		std::vector<std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>({
		std::make_shared<SparseDiscreteRandomVariable<BotState>>(std::vector<double>({ 0.9, 0.05, 0.05 }), std::vector<BotState>({ BotState::WORKING, BotState::PROPAGATING, BotState::DORMANT }))
	}));	
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-2);
	botnet_config.email_address_list_size = 10000;
	std::unique_ptr<Botnet> botnet(new Botnet(botnet_config, Sinkhole(start_date, false)));
	Context context(rng, std::move(botnet), std::move(schedule));
	ASSERT_EQ(nbr_time_points, context.get_number_time_points());
	ASSERT_EQ(nbr_time_points, context.get_schedule().size());
	for (int i = 0; i < nbr_time_points; ++i) {
		ASSERT_EQ(i, context.get_time_point_index()) << i;
		ASSERT_EQ(i * step_size_in_minutes * 60, context.get_time()) << i;
		ASSERT_EQ(context.get_time(), context.get_schedule().get_interval_from_start(i)) << i;
		ASSERT_TRUE(context.is_inside_current_time_step(context.get_time()));
		ASSERT_FALSE(context.is_inside_current_time_step(context.get_time() - 0.1));
		if (i < nbr_time_points - 1) {
			ASSERT_TRUE(context.is_inside_current_time_step(context.get_time() + 0.1));
			ASSERT_EQ((i + 1) * step_size_in_minutes * 60, context.get_next_time()) << i;
			context.make_time_step();
		} else {
			ASSERT_TRUE(is_time_undefined(context.get_next_time()));
			ASSERT_THROW(context.make_time_step(), std::logic_error);
		}
	}
}
