#include <gtest/gtest.h>
#include "simbo/fixed_host.hpp"
#include "simbo/local_network.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "mock_operating_system.hpp"

namespace simbo {
	class MockFixedHost : public FixedHost {
	public:
		MockFixedHost(id_t id, operating_system_ptr&& os, bool always_on, local_network_ptr local_network)
			: FixedHost(id, std::move(os), always_on, local_network) {}

		HostType get_type() const override {
			return HostType::DESKTOP;
		}
	};
}

using namespace simbo;

class FixedHostTest : public ::testing::Test {
protected:
	FixedHostTest()
		: local_network(1, LocalNetworkType::HOUSEHOLD, 1)
	{}

	LocalNetwork local_network;	
};

TEST_F(FixedHostTest, constructor) {
	const int id = 1;
	MockFixedHost host(id, std::make_unique<MockOS>(), false, &local_network);
	ASSERT_EQ(id, host.get_id());
	ASSERT_FALSE(host.is_always_on());
	ASSERT_EQ(&local_network, host.get_local_network());
	ASSERT_NE(nullptr, &host.get_operating_system());
	ASSERT_FALSE(host.has_internet_access());
	PublicConnectionStaticAddress public_connection(IpAddress::from_string("145.34.23.10"));
	local_network.set_public_connection(&public_connection);
	ASSERT_TRUE(host.has_internet_access());
	ASSERT_TRUE(host.is_fixed());
}

TEST_F(FixedHostTest, constructor_throws) {
	ASSERT_THROW(MockFixedHost(-10, std::make_unique<MockOS>(), false, &local_network), std::domain_error);
	ASSERT_THROW(MockFixedHost(0, std::make_unique<MockOS>(), true, &local_network), std::domain_error);
	ASSERT_THROW(MockFixedHost(10, nullptr, false, &local_network), std::invalid_argument);
	ASSERT_THROW(MockFixedHost(10, std::make_unique<MockOS>(), true, nullptr), std::invalid_argument);
}
