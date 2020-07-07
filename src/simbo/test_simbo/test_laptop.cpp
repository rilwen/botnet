#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/host_factory.hpp"
#include "simbo/laptop.hpp"
#include "simbo/local_network.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "simbo/predetermined_process.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/sinkhole.hpp"
#include "simbo/sysadmin.hpp"
#include "mock_botnet_config.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

using namespace simbo;

class LaptopTest : public ::testing::Test {
protected:
	LaptopTest() 
		: sysadmin(std::make_shared<PausingJumpMarkovProcess>(std::unique_ptr<JumpMarkovProcess>(new SimplePoissonProcess(1. / (24 * 3600))), 0)),
		t0(0.5),
		t1(4.5),
		t2(9.9),
		ln0(1, LocalNetworkType::HOUSEHOLD, 1),
		ln1(2, LocalNetworkType::CORPORATE, 1),
		local_networks({&ln0, &ln1}),
		rng(new MockRNG())
	{
		std::vector<seconds_t> times({ t0, t1, t2 });
		std::vector<int> indices({ 1, 0 });
		PredeterminedProcess<int>::values_interpolator_ptr interpolator(new PiecewiseConstantInterpolator1D<seconds_t, int>(std::move(times), std::move(indices)));
		switching_process.reset(new PredeterminedProcess<int>(std::move(interpolator)));
	}

	Laptop::local_network_switcher_t make_local_network_switcher() {
		return Laptop::local_network_switcher_t(switching_process, make_copy(local_networks), local_networks[0]);
	}

	Context make_context(std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
	}

	HostFactory factory;
	Sysadmin sysadmin;
	seconds_t t0;
	seconds_t t1;
	seconds_t t2;
	LocalNetwork ln0;
	LocalNetwork ln1;
	Laptop::local_network_switcher_t::switching_process_ptr switching_process;
	std::vector<LocalNetwork*> local_networks;
	std::shared_ptr<MockRNG> rng;	
};

// Konstruktor jest testowany w test_host_factory.cpp

TEST_F(LaptopTest, update_local_network) {
	Laptop* laptop = factory.make_laptop(sysadmin, std::make_unique<MockOS>(), make_local_network_switcher());
	const seconds_t t_mid = (t0 + t1) / 2;
	const auto t_switch = (t1 + t2) / 2;
	assert(t0 > 0);
	std::vector<seconds_t> intervals({ 0, t0, t_mid, t_switch, t2 });
	Context ctx(make_context(std::move(intervals)));
	laptop->turn_on(ctx, 0);
	ASSERT_EQ(nullptr, laptop->get_local_network());
	ASSERT_FALSE(laptop->has_internet_access());
	ctx.make_time_step(); // -> t0
	laptop->update_state(ctx);
	ASSERT_EQ(&ln1, laptop->get_local_network());
	ASSERT_FALSE(laptop->has_internet_access());
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	ln1.set_public_connection(&public_connection);
	ASSERT_TRUE(laptop->has_internet_access());
	ASSERT_EQ(t0, laptop->get_local_network_change_time());
	ctx.make_time_step(); // -> t_mid
	laptop->update_state(ctx);
	ASSERT_EQ(&ln1, laptop->get_local_network());
	ASSERT_EQ(t0, laptop->get_local_network_change_time());
	
	ctx.make_time_step(); // -> t_switch
	laptop->update_state(ctx);
	ASSERT_EQ(&ln0, laptop->get_local_network());
	ASSERT_EQ(t_switch, laptop->get_local_network_change_time());
	ctx.make_time_step(); // -> t2
	laptop->update_state(ctx);
	ASSERT_EQ(&ln0, laptop->get_local_network());
	ASSERT_EQ(t_switch, laptop->get_local_network_change_time());
}

TEST_F(LaptopTest, update_throws) {
	Laptop* laptop = factory.make_laptop(sysadmin, std::make_unique<MockOS>(), make_local_network_switcher());
	std::vector<seconds_t> intervals({ 0, t1 });
	Context ctx(make_context(std::move(intervals)));
	laptop->turn_on(ctx, 0);
	intervals = { 0., t0 };
	Context ctx2(make_context(std::move(intervals)));
	ASSERT_THROW(laptop->update_state(ctx2), std::domain_error);
	ctx.make_time_step(); // -> t1
	laptop->update_state(ctx);
	ctx2.make_time_step(); // -> t0
	ASSERT_THROW(laptop->update_state(ctx2), std::domain_error);
}
