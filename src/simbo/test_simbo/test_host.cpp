#include <gtest/gtest.h>
#include "simbo/antivirus_config.hpp"
#include "simbo/botnet.hpp"
#include "simbo/context.hpp"
#include "simbo/host.hpp"
#include "simbo/local_network.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/sinkhole.hpp"
#include "simbo/sysadmin.hpp"
#include "mock_botnet_config.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class HostTest : public ::testing::Test {
protected:
	HostTest()
		: rng(new MockRNG())
	{}

	Context make_context(std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
	}

	std::shared_ptr<MockRNG> rng;
};

TEST_F(HostTest, constructor) {
	const Host::id_t id = 10;
	MockHost host(id, std::make_unique<MockOS>(), false, true);
	ASSERT_EQ(nullptr, host.get_local_network());
	ASSERT_FALSE(host.is_always_on());
	ASSERT_FALSE(host.has_internet_access());
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_FALSE(host.is_on());
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	const TrackedValue<BotState> expected_bot_state(BotState::NONE, -infinite_time());
	ASSERT_EQ(expected_bot_state, host.get_bot_state());
	ASSERT_EQ(id, host.get_id());
	ASSERT_EQ(-infinite_time(), host.get_activity_state().get_last_update_time());
	ASSERT_EQ(-infinite_time(), host.get_infection_state().get_last_change_time());
	ASSERT_NE(nullptr, &host.get_operating_system());
	ASSERT_EQ(0, host.get_user_counter());
	ASSERT_TRUE(host.is_fixed());
	ASSERT_FALSE(host.has_antivirus());	
}

TEST_F(HostTest, constructor_always_on) {
	MockHost host(1, std::make_unique<MockOS>(), true, false);
	ASSERT_TRUE(host.is_always_on());
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_TRUE(host.is_on());
	ASSERT_FALSE(host.is_fixed());
}

TEST_F(HostTest, constructor_throws) {
	ASSERT_THROW(MockHost(-10, std::make_unique<MockOS>(), true, true), std::domain_error);
	ASSERT_THROW(MockHost(0, std::make_unique<MockOS>(), false, true), std::domain_error);
	ASSERT_THROW(MockHost(10, nullptr, false, true), std::invalid_argument);
}

TEST_F(HostTest, activity_state) {
	MockHost host(10, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({0, 1, 2, 3, 3.5, 4, 5});
	Context ctx(make_context(std::move(intervals)));
	host.turn_off(ctx, 0.001);
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(0.001, host.get_activity_state().get_last_update_time());
	ctx.make_time_step(); // -> 1
	host.turn_off(ctx, 1.01);
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(1.01, host.get_activity_state().get_last_update_time());
	ctx.make_time_step(); // -> 2
	host.turn_on(ctx, 2.2);
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(2.2, host.get_activity_state().get_last_update_time());
	ASSERT_THROW(host.turn_off(ctx, 2.), std::domain_error);
	ASSERT_THROW(host.turn_on(ctx, 2.), std::domain_error);
	ASSERT_THROW(host.turn_off(ctx, 2.2), std::domain_error);
	ASSERT_THROW(host.turn_on(ctx, 2.2), std::domain_error);
	host.turn_off(ctx, 2.21);
	return;
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(2.21, host.get_activity_state().get_last_update_time());
	host.turn_on(ctx, 2.3);
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(2.3, host.get_activity_state().get_last_update_time());
	ctx.make_time_step(); // -> 3
	host.turn_on(ctx, ctx.get_time());
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(ctx.get_time(), host.get_activity_state().get_last_update_time());
	ctx.make_time_step(); // -> 3.5
	host.turn_off(ctx, 3.5);
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(3.5, host.get_activity_state().get_last_update_time());
	ctx.make_time_step(); // -> 4
	host.turn_off(ctx, 4);
	ASSERT_EQ(4, host.get_activity_state().get_last_update_time());
}

TEST_F(HostTest, infection_state_no_internet)  {
	std::shared_ptr<RNG> rng(new MockRNG());
	std::vector<seconds_t> intervals({ 0, 1, 2, 2.5, 3, 3.5 });
	Context ctx(make_context(std::move(intervals)));
	intervals = std::vector<seconds_t>({ 0, 3 });
	Context ctx2(make_context(std::move(intervals)));
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	host.turn_on(ctx, 0);
	ctx.make_time_step(); // -> 1
	ASSERT_THROW(host.cure(ctx), std::logic_error);
	host.infect(ctx, 1.1);
	ASSERT_EQ(InfectionState::INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(1.1, host.get_infection_state().get_last_change_time());
	ASSERT_THROW(host.infect(ctx, 1.1), std::logic_error);
	ASSERT_THROW(host.infect(ctx, 1.), std::logic_error);
	ASSERT_THROW(host.cure(ctx2), std::domain_error);
	ctx.make_time_step(); // -> 2
	host.cure(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(2, host.get_infection_state().get_last_change_time());
	ctx2.make_time_step();
	ASSERT_THROW(host.cure(ctx2), std::logic_error);
	ctx.make_time_step(); // -> 2.5
	host.reset(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(2, host.get_infection_state().get_last_change_time());
	ctx.make_time_step(); // -> 3
	host.infect(ctx, 3.1);
	ASSERT_EQ(3.1, host.get_infection_state().get_last_change_time());
	ctx.make_time_step(); // -> 3.5
	host.reset(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(3.5, host.get_infection_state().get_last_change_time());
}

TEST_F(HostTest, infection_state_with_internet) {
	BotnetConfig botnet_config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	std::shared_ptr<RNG> rng(new MockRNG());
	std::vector<seconds_t> intervals({ 0, 1, 1.5, 2, 2.5 });
	Context ctx(make_context(std::move(intervals)));
	intervals = std::vector<seconds_t>({ 0, 2 });
	Context ctx2(make_context(std::move(intervals)));
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	ASSERT_THROW(host.cure(ctx), std::logic_error);
	host.infect(ctx, 0.001);
	ASSERT_EQ(InfectionState::INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(0.001, host.get_infection_state().get_last_change_time());
	ASSERT_THROW(host.infect(ctx, ctx.get_time()), std::logic_error);
	ASSERT_THROW(host.cure(ctx), std::domain_error);
	ctx.make_time_step(); // -> 1
	host.cure(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(1, host.get_infection_state().get_last_change_time());
	ctx2.make_time_step(); // -> 2
	ASSERT_THROW(host.cure(ctx2), std::logic_error);
	ctx.make_time_step(); // -> 1.5
	host.reset(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ctx.make_time_step(); // -> 2
	ASSERT_THROW(host.infect(ctx, 2.5), std::domain_error);
	ASSERT_THROW(host.infect(ctx, 25), std::domain_error);
	host.infect(ctx, 2.1);
	ASSERT_EQ(2.1, host.get_infection_state().get_last_change_time());
	ctx.make_time_step(); // -> 2.5
	host.reset(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(2.5, host.get_infection_state().get_last_change_time());
}

TEST_F(HostTest, update_state_activity) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Context ctx(make_context(std::move(intervals)));
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(-infinite_time(), host.get_activity_state().get_last_update_time());
	host.turn_on(ctx, 0);
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(0., host.get_activity_state().get_last_update_time());
	ctx.make_time_step();
	host.turn_off(ctx, 0.11);
	ASSERT_EQ(ActivityState::OFF, host.get_activity_state().get_value());
	ASSERT_EQ(0.11, host.get_activity_state().get_last_update_time());
}

TEST_F(HostTest, update_state_activity_always_on) {
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Context ctx(make_context(std::move(intervals)));
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(-infinite_time(), host.get_activity_state().get_last_update_time());
	host.turn_on(ctx, 0);
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(-infinite_time(), host.get_activity_state().get_last_update_time()); ctx.make_time_step();
	host.turn_off(ctx, 0.11);
	ASSERT_EQ(ActivityState::ON, host.get_activity_state().get_value());
	ASSERT_EQ(-infinite_time(), host.get_activity_state().get_last_update_time());
}

TEST_F(HostTest, has_internet_access) {
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	ASSERT_NE(nullptr, host.get_local_network());
	ASSERT_FALSE(host.has_internet_access());
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));	
	ln.set_public_connection(&public_connection);
	ASSERT_TRUE(host.has_internet_access());
}

TEST_F(HostTest, try_infecting_via_email_too_new_version) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Schedule schedule(std::move(intervals));
	BotnetConfig config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 0;
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	host.update_state(ctx); // Żeby włączyć hosta
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	rng->add_uniform(1.);
	host.turn_on(ctx, ctx.get_time());
	host.try_infecting_via_email(ctx, 0.001);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
}

TEST_F(HostTest, infection_development) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Schedule schedule(std::move(intervals));
	const seconds_t setting_up_period = 0.3;
	const seconds_t ping_interval = 5;
	const seconds_t reconfiguration_interval = 5;
	BotnetConfig config = make_mock_botnet_config(setting_up_period, ping_interval, reconfiguration_interval, false, false, 0.2, 0.001);
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	host.turn_on(ctx, ctx.get_time());
	rng->add_uniform(1.);
	const seconds_t infection_time = 0.001;
	host.try_infecting_via_email(ctx, infection_time);
	ASSERT_EQ(infection_time, host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_EQ(0, host.get_online_time_since_joining_botnet().get_own_time());
	ASSERT_EQ(infection_time, host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_EQ(0, host.get_uptime_since_last_ping().get_own_time());
	ASSERT_EQ(InfectionState::INFECTED, host.get_infection_state().get_value());
	ASSERT_EQ(1, host.get_payload_release_number());
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.1, ctx.get_time());
	ASSERT_EQ(BotState::SETTING_UP, host.get_bot_state().get_value());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_change_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_update_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_bot_state().get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_bot_state().get_own_time_since_last_update(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_uptime_since_last_ping().get_own_time(), 1e-14);
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.2, ctx.get_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_change_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_update_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_bot_state().get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_bot_state().get_own_time_since_last_update(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time, host.get_uptime_since_last_ping().get_own_time(), 1e-14);
	const seconds_t turn_off_time = 0.22;
	const seconds_t turn_on_time = 0.26;
	host.turn_off(ctx, turn_off_time);
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_change_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_update_time());
	ASSERT_EQ(turn_off_time, host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(turn_off_time - infection_time, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(turn_off_time, host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(turn_off_time - infection_time, host.get_uptime_since_last_ping().get_own_time(), 1e-14);
	host.turn_on(ctx, turn_on_time);
	ASSERT_EQ(turn_on_time, host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(turn_off_time - infection_time, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(turn_on_time, host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(turn_off_time - infection_time, host.get_uptime_since_last_ping().get_own_time(), 1e-14);
	const auto time_off = turn_on_time - turn_off_time;
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.3, ctx.get_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_change_time());
	ASSERT_EQ(infection_time, host.get_bot_state().get_last_update_time());
	ASSERT_NEAR(ctx.get_time() - infection_time - time_off, host.get_bot_state().get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(ctx.get_time() - infection_time - time_off, host.get_bot_state().get_own_time_since_last_update(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time - time_off, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time - time_off, host.get_uptime_since_last_ping().get_own_time(), 1e-14);	
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.4, ctx.get_time());
	const seconds_t configuration_time = ctx.get_time();
	ASSERT_GT(configuration_time, infection_time + setting_up_period);
	ASSERT_NE(BotState::SETTING_UP, host.get_bot_state().get_value());
	ASSERT_EQ(configuration_time, host.get_bot_state().get_last_change_time());
	ASSERT_EQ(configuration_time, host.get_bot_state().get_last_update_time());
	ASSERT_EQ(configuration_time, host.get_last_ping_time());
	ASSERT_NEAR(ctx.get_time() - configuration_time, host.get_bot_state().get_own_time_since_last_change(), 1e-14);
	ASSERT_NEAR(ctx.get_time() - configuration_time, host.get_bot_state().get_own_time_since_last_update(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_online_time_since_joining_botnet().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - infection_time - time_off, host.get_online_time_since_joining_botnet().get_own_time(), 1e-14);
	ASSERT_EQ(ctx.get_time(), host.get_uptime_since_last_ping().get_last_update_wall_time());
	ASSERT_NEAR(ctx.get_time() - configuration_time, host.get_uptime_since_last_ping().get_own_time(), 1e-14);
}

TEST_F(HostTest, reconfiguration_at_start) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Schedule schedule(std::move(intervals));
	const seconds_t setting_up_period = 0.01;
	const seconds_t ping_interval = 5;
	const seconds_t reconfiguration_interval = 5;
	BotnetConfig config = make_mock_botnet_config(setting_up_period, ping_interval, reconfiguration_interval, true, false, 0.2, 0.001);
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	host.turn_on(ctx, ctx.get_time());
	rng->add_uniform(1.);
	const seconds_t infection_time = 0.001;
	host.try_infecting_via_email(ctx, infection_time);
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.1, ctx.get_time());
	const seconds_t turn_off_time = 0.12;
	const seconds_t turn_on_time = 0.16;
	host.turn_off(ctx, turn_off_time);
	host.turn_on(ctx, turn_on_time);
	ASSERT_EQ(turn_on_time, host.get_bot_state().get_last_update_time());
	ASSERT_EQ(turn_on_time, host.get_last_ping_time());
}

TEST_F(HostTest, periodic_ping_and_reconfiguration) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 1, 2, 3, 4 });
	Schedule schedule(std::move(intervals));
	const seconds_t infection_time = 0.01;
	const seconds_t setting_up_period = 0.5;
	const seconds_t ping_interval = 0.5;
	const seconds_t reconfiguration_interval = 2;
	BotnetConfig config = make_mock_botnet_config(setting_up_period, ping_interval, reconfiguration_interval, false, false, 0.2, 0.001);
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	host.turn_on(ctx, ctx.get_time());
	rng->add_uniform(1.);	
	host.try_infecting_via_email(ctx, infection_time);
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(1, ctx.get_time());
	ASSERT_EQ(ctx.get_time(), host.get_bot_state().get_last_update_time());
	ASSERT_EQ(ctx.get_time(), host.get_last_ping_time());
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(2, ctx.get_time());
	ASSERT_EQ(1, host.get_bot_state().get_last_update_time());
	ASSERT_EQ(ctx.get_time(), host.get_last_ping_time());
	// Oczekujemy że sinkhole zbierze 4 wiadomości: host dołączający do botnetu, host proszący o konfigurację i 2 pingi.
	ASSERT_EQ(4, ctx.get_botnet().get_sinkhole().get_number_data_points());
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(3, ctx.get_time());
	ASSERT_EQ(ctx.get_time(), host.get_bot_state().get_last_update_time());
	ASSERT_EQ(ctx.get_time(), host.get_last_ping_time());
}

TEST_F(HostTest, ping_botnet_at_start) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	std::vector<seconds_t> intervals({ 0, 0.1, 0.2, 0.3, 0.4 });
	Schedule schedule(std::move(intervals));
	const seconds_t setting_up_period = 0.01;
	const seconds_t ping_interval = 5;
	const seconds_t reconfiguration_interval = 5;
	BotnetConfig config = make_mock_botnet_config(setting_up_period, ping_interval, reconfiguration_interval, false, true, 0.2, 0.001);
	config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(config, Sinkhole(start_date, true)), std::move(schedule));
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	host.turn_on(ctx, ctx.get_time());
	rng->add_uniform(1.);
	const seconds_t infection_time = 0.001;
	host.try_infecting_via_email(ctx, infection_time);
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(0.1, ctx.get_time());
	const seconds_t turn_off_time = 0.12;
	const seconds_t turn_on_time = 0.16;
	host.turn_off(ctx, turn_off_time);
	host.turn_on(ctx, turn_on_time);
	ASSERT_EQ(turn_on_time, host.get_last_ping_time());
}

TEST_F(HostTest, add_remove_user) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	host.add_user();
	ASSERT_EQ(1, host.get_user_counter());
	host.remove_user();
	ASSERT_EQ(0, host.get_user_counter());
}

TEST_F(HostTest, remove_user_throws) {
	MockHost host(1, std::make_unique<MockOS>(), false, true);
	ASSERT_THROW(host.remove_user(), std::logic_error);
}

TEST_F(HostTest, cured_by_antivirus) {
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	std::vector<seconds_t> intervals({ 0, 1, 2, 3, 4 });
	Schedule schedule(std::move(intervals));
	BotnetConfig botnet_config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;

	BotnetPayloadConfig payload_config = botnet_config.payload.get_ys()[0];
	payload_config.min_detecting_av_release["AV"] = 2;
	botnet_config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_config);
	
	AntivirusConfig av_config;
	av_config.infection_attempt_detection_probability = 0.;
	av_config.scan_interval = TimeDuration(0, 0, 1, 0);
	av_config.scan_after_update = true;
	av_config.update_at_startup = true;
	av_config.scan_at_startup = true;
	av_config.validate();
	Antivirus::vendor_t::config_t av_vendor_config;
	av_vendor_config.release_schedules["AV"] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>(
		std::vector<DateTime>({schedule.get_point(0), schedule.get_point(1)}), std::vector<int>({ 1, 2 }));
	auto av_vendor = std::make_shared<Antivirus::vendor_t>("AVV", av_vendor_config);
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(botnet_config, Sinkhole(start_date, true)), std::move(schedule));
	host.set_antivirus(std::make_unique<Antivirus>("AV", av_vendor, 1, av_config));
	ASSERT_TRUE(host.has_antivirus());
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	host.update_state(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
	rng->add_uniform(1.);
	host.try_infecting_via_email(ctx, 0.001);
	ASSERT_EQ(InfectionState::INFECTED, host.get_infection_state().get_value());	
	ctx.make_time_step();
	host.update_state(ctx);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
}

TEST_F(HostTest, defended_by_antivirus_heuristic) {
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	std::vector<seconds_t> intervals({ 0, 1, 2, 3, 4 });
	Schedule schedule(std::move(intervals));
	BotnetConfig botnet_config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	BotnetPayloadConfig payload_config = botnet_config.payload.get_ys()[0];
	payload_config.min_detecting_av_release["AV"] = 2;
	botnet_config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_config);
	AntivirusConfig av_config;
	av_config.infection_attempt_detection_probability = 1;
	av_config.scan_interval = TimeDuration(0, 0, 1, 0);
	av_config.scan_after_update = true;
	av_config.update_at_startup = true;
	av_config.scan_at_startup = true;
	av_config.validate();
	Antivirus::vendor_t::config_t av_vendor_config;
	av_vendor_config.release_schedules["AV"] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>(
		std::vector<DateTime>({ schedule.get_point(0), schedule.get_point(1) }), std::vector<int>({ 1, 2 }));
	auto av_vendor = std::make_shared<Antivirus::vendor_t>("AVV", av_vendor_config);
	const auto start_date = schedule.get_start();
	Context ctx(rng, std::make_unique<Botnet>(botnet_config, Sinkhole(start_date, true)), std::move(schedule));
	host.set_antivirus(std::make_unique<Antivirus>("AV", av_vendor, 1, av_config));
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	host.set_local_network(&ln);
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln.set_public_connection(&public_connection);
	rng->add_uniform(1.);
	host.try_infecting_via_email(ctx, 0.001);
	ASSERT_EQ(InfectionState::NOT_INFECTED, host.get_infection_state().get_value());
}

TEST_F(HostTest, get_payload_release_number_throws) {
	MockHost host(1, std::make_unique<MockOS>(), true, true);
	ASSERT_THROW(host.get_payload_release_number(), std::logic_error);
}