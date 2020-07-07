#include <gtest/gtest.h>
#include "simbo/local_network.hpp"
#include "simbo/public_connection_static_address.hpp"

using namespace simbo;

TEST(LocalNetwork, constructor) {
	LocalNetwork local_network(10, LocalNetworkType::HOUSEHOLD, 1);
	ASSERT_EQ(nullptr, local_network.get_public_connection());
	ASSERT_TRUE(local_network.get_hosts().empty());
	ASSERT_EQ(LocalNetworkType::HOUSEHOLD, local_network.get_type());
	ASSERT_EQ(10, local_network.get_id());
}

TEST(LocalNetwork, set_public_connection) {
	LocalNetwork local_network(1, LocalNetworkType::CORPORATE, 1);
	IpAddress ip_address = IpAddress::from_string("192.168.0.1");
	PublicConnectionStaticAddress pc(ip_address);
	local_network.set_public_connection(&pc);
	ASSERT_EQ(&pc, local_network.get_public_connection());
}
