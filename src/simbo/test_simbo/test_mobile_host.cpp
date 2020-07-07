#include <gtest/gtest.h>
#include "simbo/context.hpp"
#include "simbo/local_network.hpp"
#include "simbo/mobile_host.hpp"
#include "simbo/sinkhole.hpp"
#include "mock_botnet_config.hpp"
#include "mock_operating_system.hpp"
#include "mock_rng.hpp"

namespace simbo {
	class MockMobileHost : public MobileHost {
	public:
		MockMobileHost(id_t id, operating_system_ptr&& os)
			: MobileHost(id, std::move(os)) {}

		void set_new_local_network(local_network_ptr ln) {
			new_local_network_ = ln;
		}

		HostType get_type() const override {
			return HostType::LAPTOP;
		}
	private:
		MockMobileHost::local_network_ptr update_local_network(Context&) override {
			return new_local_network_;
		}
		local_network_ptr new_local_network_;
	};
}

using namespace simbo;

class MobileHostTest : public ::testing::Test {
protected:
	MobileHostTest()
		: rng(new MockRNG())
	{}

	Context make_context(std::vector<seconds_t>&& intervals) {
		Schedule schedule(std::move(intervals));
		const auto start_date = schedule.get_start();
		return Context(rng, std::make_unique<Botnet>(make_mock_botnet_config(1, 1, 5, true, false, 0.2, 0.001), Sinkhole(start_date, true)), std::move(schedule));
	}

	std::shared_ptr<MockRNG> rng;
};

TEST_F(MobileHostTest, constructor) {
	const int id = 1;
	MockMobileHost host(id, std::make_unique<MockOS>());
	ASSERT_EQ(id, host.get_id());
	ASSERT_NE(nullptr, &host.get_operating_system());
	ASSERT_FALSE(host.is_fixed());
}

TEST_F(MobileHostTest, constructor_throws) {
	ASSERT_THROW(MockMobileHost(-10, std::make_unique<MockOS>()), std::domain_error);
	ASSERT_THROW(MockMobileHost(0, std::make_unique<MockOS>()), std::domain_error);
	ASSERT_THROW(MockMobileHost(10, nullptr), std::invalid_argument);
}

TEST_F(MobileHostTest, update_state) {
	LocalNetwork ln(1, LocalNetworkType::HOUSEHOLD, 1);
	MockMobileHost host(1, std::make_unique<MockOS>());
	host.set_new_local_network(&ln);
	std::vector<seconds_t> intervals({ 0, 0.2, 0.3 });
	Context ctx(make_context(std::move(intervals)));
	host.turn_on(ctx, 0);
	host.update_state(ctx);
	ASSERT_EQ(&ln, host.get_local_network());
	ASSERT_EQ(0., host.get_local_network_change_time());
	ASSERT_EQ(1u, ln.get_hosts().size());
	ASSERT_EQ(1u, ln.get_hosts().count(&host));
	host.set_new_local_network(&ln);
	ctx.make_time_step(); // -> 0.2
	host.update_state(ctx);
	ASSERT_EQ(0., host.get_local_network_change_time());
	host.set_new_local_network(nullptr);
	ctx.make_time_step(); // -> 0.3
	host.update_state(ctx);
	ASSERT_EQ(0.3, host.get_local_network_change_time());
	ASSERT_TRUE(ln.get_hosts().empty());
}

TEST_F(MobileHostTest, switch_local_networks) {
	LocalNetwork ln1(1, LocalNetworkType::HOUSEHOLD, 1);
	LocalNetwork ln2(2, LocalNetworkType::CORPORATE, 1);
	MockMobileHost host(1, std::make_unique<MockOS>());
	host.set_new_local_network(&ln1);
	std::vector<seconds_t> intervals({ 0, 0.2, 0.3 });
	Context ctx(make_context(std::move(intervals)));
	host.turn_on(ctx, 0);
	host.update_state(ctx);
	ASSERT_EQ(1, ln1.get_hosts().count(&host));
	ASSERT_EQ(0, ln2.get_hosts().count(&host));
	host.set_new_local_network(&ln2);
	ctx.make_time_step(); // -> 0.1
	host.update_state(ctx);
	ASSERT_EQ(0, ln1.get_hosts().count(&host));
	ASSERT_EQ(1, ln2.get_hosts().count(&host));
}
