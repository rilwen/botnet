#include <gtest/gtest.h>
#include "simbo/constant_process.hpp"
#include "simbo/context.hpp"
#include "simbo/desktop.hpp"
#include "simbo/email_provider.hpp"
#include "simbo/host.hpp"
#include "simbo/host_factory.hpp"
#include "simbo/local_network.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/predetermined_process.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/simulation_controller.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "simbo/utils.hpp"
#include "simbo/world.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class SimulationControllerTest : public ::testing::Test {
protected:
	SimulationControllerTest();

	Context make_context() const;

	const int nbr_time_points = 4;
	const int step_size_in_minutes = 2;
	BotnetConfig botnet_config;
	DateTime start_date;
	TimeDuration period;
	std::shared_ptr<MockRNG> rng;
	std::shared_ptr<PausingJumpMarkovProcess> maintenance_trigger;
	HostFactory factory;
	PublicConnectionStaticAddress public_connection;
	World world;
	LocalNetwork ln;
	Sysadmin sysadmin;
	std::vector<EmailAccount> email_accounts;
	Desktop* desktop;
	Context ctx;
	OutputConfig output_config;
};

SimulationControllerTest::SimulationControllerTest()
	: start_date(Date(1970, 1, 1), TimeDuration()),
	period(0, step_size_in_minutes, 0, 0),
	rng(new MockRNG()),
	maintenance_trigger(std::make_shared<PausingJumpMarkovProcess>(std::unique_ptr<JumpMarkovProcess>(new SimplePoissonProcess(1. / (24 * 3600))), 0)),
	public_connection(IpAddress::from_string("145.34.23.10")),
	ln(1, LocalNetworkType::HOUSEHOLD, 1),
	sysadmin(maintenance_trigger)
{
	botnet_config.dropper.setting_up_period = 240;
	botnet_config.probability_of_reconfiguration = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 0.9);

	BotnetPayloadConfig payload_config;
	payload_config.ping_interval = ActionInterval(480, false);
	payload_config.reconfiguration_interval = ActionInterval(720, true);
	payload_config.reconfigure_at_start = false;
	payload_config.infectious_emails_sent_per_second_per_bot = 0;

	botnet_config.payload = PiecewiseConstantInterpolator1D<int, BotnetPayloadConfig>(1, 2, payload_config);
	botnet_config.bot_state_weights = PiecewiseConstantInterpolator1D<DateTime, std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>(
		std::vector<DateTime>({start_date}),
		std::vector<std::shared_ptr<SparseDiscreteRandomVariable<BotState>>>({
		std::make_shared<SparseDiscreteRandomVariable<BotState>>(std::vector<double>({0.9, 0.05, 0.05}), std::vector<BotState>({BotState::WORKING, BotState::PROPAGATING, BotState::DORMANT}))
	}));
	
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-2);
	botnet_config.email_address_list_size = 1000;
	botnet_config.payload_release_schedule = PiecewiseConstantInterpolator1D<DateTime, int>(min_datetime(), max_datetime(), 1);

	ln.set_public_connection(&public_connection);

	desktop = factory.make_desktop(sysadmin, std::make_unique<MockOS>(), true, &ln);

	std::shared_ptr<const EmailProvider> email_provider(new EmailProvider("poczta", 0));
	email_accounts.push_back(EmailAccount(email_provider, EmailAccount::check_process_ptr(new SimplePoissonProcess(0.1)), EmailAccount::hosts_set(), EmailAccount::local_networks_set()
	));

	world.add_sysadmin(std::move(sysadmin));

	rng->add_uniform(0.1);

	output_config.network_structure_filename = "__test__network.csv";
	output_config.sinkhole_data_filename = "__test__sinkhole.bin";
}

Context SimulationControllerTest::make_context() const {
	Schedule schedule(start_date, period, nbr_time_points);
	const auto start_date = schedule.get_start();
	return Context(rng, std::unique_ptr<Botnet>(new Botnet(botnet_config, Sinkhole(start_date, true))), std::move(schedule));
}

TEST_F(SimulationControllerTest, run_empty_world) {
	Context ctx(make_context());
	world = World();
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(simcon.get_context().get_number_time_points(), simcon.get_context().get_time_point_index() + 1);
}

TEST_F(SimulationControllerTest, run_no_users) {
	Context ctx(make_context());	
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	rng->clear();
	simcon.run();
	ASSERT_EQ(InfectionState::NOT_INFECTED, desktop->get_infection_state().get_value());
}

TEST_F(SimulationControllerTest, run_host_user) {
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-1);
	botnet_config.email_address_list_size = 1;
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	Context ctx(make_context());
	User::host_switcher_t host_switcher(std::shared_ptr<MarkovProcess<int>>(new ConstantProcess<int>(0)), std::vector<Host*>({ desktop }), desktop);
	User user(std::move(host_switcher), std::move(email_accounts), 1, 1, 1.5, 0);
	world.add_user(std::move(user));
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(InfectionState::INFECTED, desktop->get_infection_state().get_value());
}

TEST_F(SimulationControllerTest, run_host_user_does_not_open) {
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-1);
	botnet_config.email_address_list_size = 1;
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	Context ctx(make_context());
	User::host_switcher_t host_switcher(std::shared_ptr<MarkovProcess<int>>(new ConstantProcess<int>(0)), std::vector<Host*>({ desktop }), desktop);
	User user(std::move(host_switcher), std::move(email_accounts), 1, 0, 1.5, 0);
	world.add_user(std::move(user));
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(InfectionState::NOT_INFECTED, desktop->get_infection_state().get_value());
}

TEST_F(SimulationControllerTest, run_host_user_does_not_click) {
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-1);
	botnet_config.email_address_list_size = 1;
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	Context ctx(make_context());
	User::host_switcher_t host_switcher(std::shared_ptr<MarkovProcess<int>>(new ConstantProcess<int>(0)), std::vector<Host*>({ desktop }), desktop);
	User user(std::move(host_switcher), std::move(email_accounts), 0, 1, 1.5, 0);
	world.add_user(std::move(user));
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(InfectionState::NOT_INFECTED, desktop->get_infection_state().get_value());
}

TEST_F(SimulationControllerTest, run_host_user_does_not_check) {
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-1);
	botnet_config.email_address_list_size = 1;
	Context ctx(make_context());
	User::host_switcher_t host_switcher(std::shared_ptr<MarkovProcess<int>>(new ConstantProcess<int>(0)), std::vector<Host*>({ desktop }), desktop);
	User user(std::move(host_switcher), std::move(email_accounts), 1, 1, 1.5, 0);
	world.add_user(std::move(user));
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(InfectionState::NOT_INFECTED, desktop->get_infection_state().get_value());
}

TEST_F(SimulationControllerTest, run_host_no_internet_user) {
	botnet_config.infectious_email_sent_per_second_by_command_centre = PiecewiseConstantInterpolator1D<DateTime, double>(min_datetime(), max_datetime(), 1e-1);
	botnet_config.email_address_list_size = 1;
	Context ctx(make_context());
	ln.set_public_connection(nullptr);
	User::host_switcher_t host_switcher(std::shared_ptr<MarkovProcess<int>>(new ConstantProcess<int>(0)), std::vector<Host*>({ desktop }), desktop);	
	User user(std::move(host_switcher), std::move(email_accounts), 1, 1, 1.5, 0);
	world.add_user(std::move(user));
	SimulationController simcon(std::move(world), std::move(ctx), output_config);
	simcon.run();
	ASSERT_EQ(InfectionState::NOT_INFECTED, desktop->get_infection_state().get_value());
}
