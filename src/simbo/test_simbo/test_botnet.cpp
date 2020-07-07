#include <gtest/gtest.h>
#include "simbo/botnet.hpp"
#include "simbo/botnet_config.hpp"
#include "simbo/context.hpp"
#include "simbo/local_network.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/schedule.hpp"
#include "simbo/sinkhole.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "simbo/windows.hpp"
#include "mock_botnet_config.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

TEST(Botnet, is_infectable) {
	BotnetPayloadConfig payload_cfg;
	payload_cfg.ping_interval = ActionInterval(300, false);
	payload_cfg.reconfiguration_interval = ActionInterval(3600, true);
	payload_cfg.reconfigure_at_start = false;
	payload_cfg.infectious_emails_sent_per_second_per_bot = 0.05;

	BotnetConfig config;
	config.dropper.setting_up_period = 60;
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::WINDOWS, 7)] = 2;
	config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_cfg);
	config.probability_of_reconfiguration = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 0.1);
	config.bot_state_weights = PiecewiseConstantInterpolator1D<DateTime, std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>(
		std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration(8, 0, 0, 0)) }),
		std::vector<std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>({
		std::make_shared<SparseDiscreteRandomVariable<BotState>>(std::vector<double>({ 0.5, 0.25, 0.25 }), std::vector<BotState>({ BotState::WORKING, BotState::PROPAGATING, BotState::DORMANT }))
	}));
	config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e2);
	config.email_address_list_size = 2000;
	config.payload_release_schedule = PiecewiseConstantInterpolator1D<DateTime, int>(min_datetime(), max_datetime(), 1);
	
	Botnet botnet(config, Sinkhole(Sinkhole::EPOCH, true));
	OperatingSystem::vendor_t::config_t os_vendor_cfg;
	os_vendor_cfg.release_schedules[std::make_pair(OperatingSystemType::WINDOWS, 7)] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>();
	os_vendor_cfg.release_schedules[std::make_pair(OperatingSystemType::WINDOWS, 10)] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>();
	auto os_vendor = std::make_shared<OperatingSystem::vendor_t>("LittleSoftie", os_vendor_cfg);
	Windows os_a(os_vendor, 7, 2);
	Windows os_b(os_vendor, 7, 3);
	Windows os_c(os_vendor, 10, 1);
	ASSERT_TRUE(botnet.is_infectable(os_a, nullptr));
	ASSERT_FALSE(botnet.is_infectable(os_b, nullptr));
	ASSERT_FALSE(botnet.is_infectable(os_c, nullptr));
}

TEST(Botnet, configure_host_setting_up) {
	BotnetConfig config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	config.probability_of_reconfiguration = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 0.);	
	
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	Schedule schedule(std::vector<double>({0, 1}));
	const auto start_date = schedule.get_start();
	std::shared_ptr<MockRNG> rng(new MockRNG);
	rng->add_uniform(0.2);
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	host.infect(ctx, 0.1);
	ASSERT_EQ(BotState::SETTING_UP, host.get_bot_state().get_value());
	const auto new_state = ctx.get_botnet().configure_host(ctx, &host, 0.2);
	ASSERT_EQ(BotState::WORKING, new_state);
}