#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/email_account.hpp"
#include "simbo/email_provider.hpp"
#include "simbo/local_network.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/sinkhole.hpp"
#include "mock_botnet_config.hpp"
#include "mock_host.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class EmailAccountTest : public ::testing::Test {
protected:
	EmailAccountTest()
		: email_filtering_efficiency(0.95),
		rng(new MockRNG()),
		email_provider(new EmailProvider("poczta", email_filtering_efficiency)),
		check_process(new SimplePoissonProcess(0.1)),
		host(1, Host::operating_system_ptr(new MockOS()), false, true) {
	}

	Context make_context(std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
	}

	double email_filtering_efficiency;
	std::shared_ptr<MockRNG> rng;
	EmailAccount::email_provider_ptr email_provider;
	EmailAccount::check_process_ptr check_process;
	MockHost host;
};

TEST_F(EmailAccountTest, constructor_all_hosts_and_networks) {
	EmailAccount email_account(email_provider, check_process, EmailAccount::hosts_set(), EmailAccount::local_networks_set());
	ASSERT_EQ(email_provider.get(), &email_account.get_email_provider());
	ASSERT_TRUE(email_account.get_allowed_hosts().empty());
	ASSERT_EQ(0, email_account.get_number_unopened_infectious_emails());
}

TEST_F(EmailAccountTest, constructor_allowed_host) {
	EmailAccount email_account(email_provider, check_process, EmailAccount::hosts_set({ &host }), EmailAccount::local_networks_set());
	ASSERT_EQ(email_provider.get(), &email_account.get_email_provider());
	ASSERT_EQ(1, email_account.get_allowed_hosts().size());
	ASSERT_EQ(&host, *email_account.get_allowed_hosts().begin());
	ASSERT_EQ(0, email_account.get_number_unopened_infectious_emails());
}

TEST_F(EmailAccountTest, constructor_throws) {
	ASSERT_THROW(EmailAccount(email_provider, check_process, EmailAccount::hosts_set({ nullptr }), EmailAccount::local_networks_set()), std::invalid_argument);
	ASSERT_THROW(EmailAccount(email_provider, check_process, EmailAccount::hosts_set(), EmailAccount::local_networks_set({ nullptr })), std::invalid_argument);
	ASSERT_THROW(EmailAccount(nullptr, check_process, std::unordered_set<EmailAccount::host_ptr>({ &host }), EmailAccount::local_networks_set()), std::invalid_argument);
	ASSERT_THROW(EmailAccount(email_provider, nullptr, std::unordered_set<EmailAccount::host_ptr>({ &host }), EmailAccount::local_networks_set()), std::invalid_argument);
}

TEST_F(EmailAccountTest, receive_infectious_email) {
	EmailAccount email_account(email_provider, check_process, EmailAccount::hosts_set({ &host }), EmailAccount::local_networks_set());
	rng->add_uniform(email_filtering_efficiency - 0.001);
	rng->add_uniform(email_filtering_efficiency + 0.001);
	const seconds_t t = 0.45;
	Context ctx(make_context(std::vector<seconds_t>({ 0, 2 * t })));
	email_account.receive_infectious_email(ctx, t);
	ASSERT_EQ(0, email_account.get_number_unopened_infectious_emails());
	email_account.receive_infectious_email(ctx, t);
	ASSERT_EQ(1, email_account.get_number_unopened_infectious_emails());
}

TEST_F(EmailAccountTest, is_host_allowed) {
	LocalNetwork ln1(1, LocalNetworkType::CORPORATE, 1);
	LocalNetwork ln2(1, LocalNetworkType::HOUSEHOLD, 1);
	EmailAccount email_account(email_provider, check_process, EmailAccount::hosts_set({ &host }), EmailAccount::local_networks_set({ &ln1 }));
	ASSERT_THROW(email_account.is_host_allowed(nullptr), std::invalid_argument);
	ASSERT_EQ(nullptr, host.get_local_network());
	ASSERT_THROW(email_account.is_host_allowed(&host), std::invalid_argument);
	host.set_local_network(&ln2);
	ASSERT_FALSE(email_account.is_host_allowed(&host));
	host.set_local_network(&ln1);
	ASSERT_TRUE(email_account.is_host_allowed(&host));
}
