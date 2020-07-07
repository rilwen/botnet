#include <gtest/gtest.h>
#include "simbo/constant_process.hpp"
#include "simbo/context.hpp"
#include "simbo/desktop.hpp"
#include "simbo/host_factory.hpp"
#include "simbo/laptop.hpp"
#include "simbo/local_network.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/server.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/sinkhole.hpp"
#include "simbo/sysadmin.hpp"
#include "mock_botnet_config.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class HostFactoryTest : public ::testing::Test {
protected:
	HostFactoryTest()
		: sysadmin(std::make_shared<PausingJumpMarkovProcess>(std::unique_ptr<JumpMarkovProcess>(new SimplePoissonProcess(1. / (24 * 3600))), 0)),
		rng(new MockRNG())
	{
	}

	Context make_context(std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
	}

	HostFactory factory;
	Sysadmin sysadmin;
	std::shared_ptr<MockRNG> rng;
};

TEST_F(HostFactoryTest, make_desktop) {	
	LocalNetwork local_network(1, LocalNetworkType::HOUSEHOLD, 1);
	Desktop* desktop = factory.make_desktop(sysadmin, std::make_unique<MockOS>(), false, &local_network);
	ASSERT_NE(nullptr, desktop);
	ASSERT_EQ(1, local_network.get_hosts().size());
	ASSERT_EQ(1, local_network.get_hosts().count(desktop));
	ASSERT_EQ(Id::min_good_id(), desktop->get_id());
	ASSERT_EQ(1, sysadmin.get_hosts().size());
	ASSERT_FALSE(desktop->is_always_on());
	ASSERT_FALSE(desktop->is_on());
	ASSERT_EQ(desktop, sysadmin.get_hosts()[0].get());
	ASSERT_NE(nullptr, &desktop->get_operating_system());
	Desktop* desktop2 = factory.make_desktop(sysadmin, std::make_unique<MockOS>(), true, &local_network);
	ASSERT_NE(nullptr, desktop2);
	ASSERT_TRUE(desktop2->is_always_on());
	ASSERT_TRUE(desktop2->is_on());
	ASSERT_EQ(desktop->get_id() + 1, desktop2->get_id());
	ASSERT_TRUE(desktop->is_fixed());
	ASSERT_EQ(HostType::DESKTOP, desktop->get_type());
}

TEST_F(HostFactoryTest, make_server) {
	LocalNetwork local_network(1, LocalNetworkType::CORPORATE, 1);
	Server* server = factory.make_server(sysadmin, std::make_unique<MockOS>(), &local_network);
	ASSERT_NE(nullptr, server);
	ASSERT_EQ(1, local_network.get_hosts().size());
	ASSERT_EQ(1, local_network.get_hosts().count(server));
	ASSERT_EQ(Id::min_good_id(), server->get_id());
	ASSERT_EQ(1, sysadmin.get_hosts().size());
	ASSERT_TRUE(server->is_always_on());
	ASSERT_TRUE(server->is_on());
	ASSERT_EQ(server, sysadmin.get_hosts()[0].get());
	ASSERT_NE(nullptr, &server->get_operating_system());	
	ASSERT_EQ(HostType::SERVER, server->get_type());
}

TEST_F(HostFactoryTest, make_laptop) {
	LocalNetwork local_network(1, LocalNetworkType::HOUSEHOLD, 1);
	std::vector<LocalNetwork*> local_networks({ &local_network });
	Laptop::local_network_switcher_t::switching_process_ptr swproc(new ConstantProcess<int>(0));
	Laptop::local_network_switcher_t lnsw(swproc, std::move(local_networks), &local_network);
	Laptop* laptop = factory.make_laptop(sysadmin, std::make_unique<MockOS>(), std::move(lnsw));
	ASSERT_NE(nullptr, laptop);
	ASSERT_EQ(Id::min_good_id(), laptop->get_id());
	ASSERT_EQ(1, sysadmin.get_hosts().size());
	ASSERT_EQ(laptop, sysadmin.get_hosts()[0].get());
	ASSERT_NE(nullptr, &laptop->get_operating_system());
	ASSERT_EQ(nullptr, laptop->get_local_network());
	auto ln_view = laptop->get_local_networks_view();
	ASSERT_EQ(1, ln_view.size());
	ASSERT_EQ(&local_network, ln_view[0]);
	Context ctx(make_context(std::vector<seconds_t>({ 0., 0.1 })));
	laptop->turn_on(ctx, 0);
	laptop->update_state(ctx);
	ASSERT_EQ(&local_network, laptop->get_local_network());
	ASSERT_EQ(0., laptop->get_local_network_change_time());
	ASSERT_FALSE(laptop->is_fixed());
	ASSERT_EQ(HostType::LAPTOP, laptop->get_type());
}
