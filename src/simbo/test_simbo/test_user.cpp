#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/email_provider.hpp"
#include "simbo/local_network.hpp"
#include "simbo/monotonic_linear_interpolator1d.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/poisson_process.hpp"
#include "simbo/predetermined_process.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/sinkhole.hpp"
#include "simbo/user.hpp"
#include "mock_botnet_config.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class UserTest : public ::testing::Test {
protected:
	UserTest():
		t_user_arrives_to_work(0),
		t_user_leaves_work(1),
		t_laptop_turned_on(3),
		t_laptop_turned_off(4),
		work_pc(
			1,
			Host::operating_system_ptr(new MockOS()),
			true,
			true
		),
		home_laptop(
			2, 
			Host::operating_system_ptr(new MockOS()),
			false,
			false
		),
		rng(new MockRNG()),
		personal_email_filtering_efficiency(0.95),
		personal_email_provider(new EmailProvider("home email", personal_email_filtering_efficiency)),
		work_email_provider(new EmailProvider("work email", 0.999)),
		personal_email_check_lambda(2),
		allowed_hosts_personal({&home_laptop}),
		work_email_check_lambda(0.5),
		allowed_hosts_work({ &work_pc }),
		home_local_network(1, LocalNetworkType::HOUSEHOLD, 1),
		home_public_connection(IpAddress::from_string("247.34.23.20"))
	{
		home_local_network.set_public_connection(&home_public_connection);
		home_laptop.set_local_network(&home_local_network);
	}

	EmailAccount::check_process_ptr make_check_process_personal(double lambda) {
		return EmailAccount::check_process_ptr(new PoissonProcess(
			PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(
				std::vector<seconds_t>({ t_user_arrives_to_work, t_laptop_turned_off }),
				std::vector<double>({ lambda, 0 }),
				0
				))
		));
	}

	EmailAccount::check_process_ptr make_check_process_work(double lambda) {
		return EmailAccount::check_process_ptr(new PoissonProcess(
			PoissonProcess::interpolator1d_ptr(new MonotonicLinearInterpolator1D<seconds_t, double>(
				std::vector<seconds_t>({ t_user_arrives_to_work, t_user_leaves_work }),
				std::vector<double>({ lambda, 0 }),
				0
				))
		));
	}

	EmailAccount make_personal_email_account(double lambda) {
		return EmailAccount(personal_email_provider, make_check_process_personal(lambda), make_copy(allowed_hosts_personal), EmailAccount::local_networks_set());
	}

	EmailAccount make_work_email_account(double lambda) {
		return EmailAccount(work_email_provider, make_check_process_work(lambda), make_copy(allowed_hosts_work), EmailAccount::local_networks_set());
	}

	std::vector<EmailAccount> make_email_accounts(bool only_instantaneous_checks) {
		std::vector<EmailAccount> accounts;
		accounts.push_back(make_personal_email_account(only_instantaneous_checks ? 0 : personal_email_check_lambda));
		accounts.push_back(make_work_email_account(only_instantaneous_checks ? 0 : work_email_check_lambda));
		return accounts;
	}

	MarkovSwitcher<User::host_ptr> make_host_switcher() {
		return MarkovSwitcher<User::host_ptr>(
			std::shared_ptr<MarkovProcess<int>>(new PredeterminedProcess<int>(
				PredeterminedProcess<int>::values_interpolator_ptr(new PiecewiseConstantInterpolator1D<seconds_t, int>(
					std::vector<seconds_t>({ t_user_arrives_to_work, t_user_leaves_work, t_laptop_turned_on, t_laptop_turned_off }),
					std::vector<int>({ 0, 2, 1, 2 })
					)
				))),
			std::vector<User::host_ptr>({ &work_pc, &home_laptop, nullptr }),
			nullptr);
	}

	Context make_context(std::vector<seconds_t>&& intervals) {
		return make_context(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), std::move(intervals));
	}

	Context make_context(const BotnetConfig& botnet_config, std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(botnet_config, Sinkhole(start_date, true)), std::move(schedule));
	}

	seconds_t t_user_arrives_to_work;
	seconds_t t_user_leaves_work;
	seconds_t t_laptop_turned_on;
	seconds_t t_laptop_turned_off;
	MockHost work_pc;
	MockHost home_laptop;
	std::shared_ptr<MockRNG> rng;
	double personal_email_filtering_efficiency;
	std::shared_ptr<EmailProvider> personal_email_provider;
	std::shared_ptr<EmailProvider> work_email_provider;
	double personal_email_check_lambda;
	EmailAccount::hosts_set allowed_hosts_personal;
	double work_email_check_lambda;
	EmailAccount::hosts_set allowed_hosts_work;
	LocalNetwork home_local_network;
	PublicConnectionStaticAddress home_public_connection;
};

TEST_F(UserTest, constructor) {
	User user(make_host_switcher(), make_email_accounts(false), 1, 1, 1, 0);
	ASSERT_EQ(nullptr, user.get_used_host());
	ASSERT_EQ(2, user.get_email_accounts().size());
}

TEST_F(UserTest, email_accounts) {
	User user(make_host_switcher(), make_email_accounts(false), 1, 1, 1, 0);
	ASSERT_EQ(&user.get_email_accounts()[0], &user.get_email_account(0));
	ASSERT_EQ(&user.get_email_accounts()[1], &user.get_email_account(1));
	ASSERT_EQ(&(*user.get_email_accounts_begin()), &user.get_email_account(0));
}

TEST_F(UserTest, update_used_host) {
	User user(make_host_switcher(), make_email_accounts(false), 1, 1, 1, 0);
	std::vector<seconds_t> intervals({
		t_user_arrives_to_work,
		t_user_arrives_to_work + 0.001,
		t_user_leaves_work,
		t_laptop_turned_on,
		t_laptop_turned_off - 0.001,
		t_laptop_turned_off
	});
	Context ctx(make_context(std::move(intervals)));
	user.update_used_host(ctx);
	ASSERT_EQ(&work_pc, user.get_used_host());
	ASSERT_EQ(t_user_arrives_to_work, user.get_used_host_change_time());

	ctx.make_time_step();
	user.update_used_host(ctx);
	ASSERT_EQ(&work_pc, user.get_used_host());
	ASSERT_EQ(t_user_arrives_to_work, user.get_used_host_change_time());

	ctx.make_time_step();
	user.update_used_host(ctx);
	ASSERT_EQ(nullptr, user.get_used_host());
	ASSERT_EQ(t_user_leaves_work, user.get_used_host_change_time());

	ctx.make_time_step();
	user.update_used_host(ctx);
	ASSERT_EQ(&home_laptop, user.get_used_host());
	ASSERT_EQ(t_laptop_turned_on, user.get_used_host_change_time());

	ctx.make_time_step();
	user.update_used_host(ctx);
	ASSERT_EQ(&home_laptop, user.get_used_host());
	ASSERT_EQ(t_laptop_turned_on, user.get_used_host_change_time());

	ctx.make_time_step();
	user.update_used_host(ctx);
	ASSERT_EQ(nullptr, user.get_used_host());
	ASSERT_EQ(t_laptop_turned_off, user.get_used_host_change_time());
}

TEST_F(UserTest, check_email_vewy_vewy_careful) {
	// Przypadek bardzo ostrożnego użytkownika.
	User user(make_host_switcher(), make_email_accounts(false), 0, 0, 1, 0);
	rng->add_uniform(personal_email_filtering_efficiency + 0.001);
	rng->add_uniform(0.01);
	std::vector<seconds_t> intervals({
		t_user_arrives_to_work,
		t_laptop_turned_on,
		t_laptop_turned_off
	});
	Context ctx(make_context(std::move(intervals)));
	// Konto 0: osobiste; konto 1: pracowe
	user.get_email_account(0).receive_infectious_email(ctx, t_user_arrives_to_work);
	user.get_email_account(1).receive_infectious_email(ctx, t_user_arrives_to_work);
	ASSERT_EQ(1, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ASSERT_EQ(0, user.get_email_accounts()[1].get_number_unopened_infectious_emails());
	ASSERT_EQ(2, rng->get_number_draws());
	ctx.make_time_step(); // -> t_laptop_turned_on
	user.update_used_host(ctx);
	user.get_used_host()->update_state(ctx);
	rng->add_uniform(0.5);
	rng->add_uniform(0.001);
	ASSERT_EQ(1, user.check_email(ctx));
	ASSERT_EQ(4, rng->get_number_draws());
	ASSERT_EQ(1, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ASSERT_EQ(0, user.get_email_accounts()[1].get_number_unopened_infectious_emails());
}

TEST_F(UserTest, check_email_realistic) {
	// Przypadek bardzo nieostrożnego użytkownika.
	User user(make_host_switcher(), make_email_accounts(false), 1, 1, 1, 0);
	rng->add_uniform(personal_email_filtering_efficiency + 0.001);
	std::vector<seconds_t> intervals({
		t_user_arrives_to_work,
		t_laptop_turned_on,
		t_laptop_turned_off
	});
	BotnetConfig botnet_config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	Context ctx(make_context(botnet_config, std::move(intervals)));
	// Konto 0: osobiste
	user.get_email_account(0).receive_infectious_email(ctx, t_user_arrives_to_work);
	ASSERT_EQ(1, rng->get_number_draws());
	ASSERT_EQ(1, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ctx.make_time_step(); // -> t_laptop_turned_on
	user.update_used_host(ctx);
	user.get_used_host()->update_state(ctx);
	rng->clear();
	rng->add_uniform(0.5);
	ASSERT_TRUE(user.get_used_host()->has_internet_access());
	ASSERT_EQ(1, user.check_email(ctx));
	ASSERT_EQ(3, rng->get_number_draws());
	ASSERT_EQ(0, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ASSERT_EQ(InfectionState::INFECTED, home_laptop.get_infection_state().get_value());
	ASSERT_LE(t_laptop_turned_on, home_laptop.get_infection_state().get_last_change_time());
	ASSERT_GE(t_laptop_turned_off, home_laptop.get_infection_state().get_last_change_time());	
}

TEST_F(UserTest, check_email_instantenous_checks) {
	User user(make_host_switcher(), make_email_accounts(true), 1, 1, 1, 1);	
	std::vector<seconds_t> intervals({
		t_user_arrives_to_work,
		t_laptop_turned_on,
		t_laptop_turned_off
	});
	BotnetConfig botnet_config = make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001);
	botnet_config.dropper.max_infected_release[std::make_pair(OperatingSystemType::OTHER, 1)] = 1;
	Context ctx(make_context(botnet_config, std::move(intervals)));	
	
	ctx.make_time_step(); // -> t_laptop_turned_on
	// Konto 0: osobiste	
	user.update_used_host(ctx);
	user.get_used_host()->update_state(ctx);
	ASSERT_TRUE(user.get_used_host()->has_internet_access());
	rng->add_uniform(personal_email_filtering_efficiency + 0.001);
	user.get_email_account(0).receive_infectious_email(ctx, t_laptop_turned_on);	
	rng->clear();
	rng->add_uniform(0.5);
	ASSERT_EQ(0, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ASSERT_EQ(InfectionState::INFECTED, home_laptop.get_infection_state().get_value());
	ASSERT_EQ(t_laptop_turned_on, home_laptop.get_infection_state().get_last_change_time());	
}

TEST_F(UserTest, check_email_no_internet_access) {
	User user(make_host_switcher(), make_email_accounts(false), 1, 1, 1, 0);
	rng->add_uniform(personal_email_filtering_efficiency + 0.001);
	std::vector<seconds_t> intervals({
		t_user_arrives_to_work,
		t_laptop_turned_on,
		t_laptop_turned_off
	});
	Context ctx(make_context(std::move(intervals)));
	// Konto 0: osobiste
	user.get_email_account(0).receive_infectious_email(ctx, t_user_arrives_to_work);
	ASSERT_EQ(1, rng->get_number_draws());
	ASSERT_EQ(1, user.get_email_accounts()[0].get_number_unopened_infectious_emails());
	ctx.make_time_step(); // -> t_user_comes_home
	user.update_used_host(ctx);
	ctx.make_time_step(); // -> t_user_comes_home
	user.get_used_host()->update_state(ctx);
	rng->clear();
	rng->add_uniform(0.5);
	home_laptop.set_local_network(nullptr);
	ASSERT_FALSE(user.get_used_host()->has_internet_access());
	ASSERT_EQ(0, user.check_email(ctx));
}
