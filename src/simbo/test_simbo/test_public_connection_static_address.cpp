#include <gtest/gtest.h>
#include "simbo/internet_service_provider.hpp"
#include "simbo/public_connection_static_address.hpp"
#include "simbo/unique_ip_address_pool.hpp"

using namespace simbo;

TEST(PublicConnectionStaticAddress, constructor) {
	IpAddress ip_address = IpAddress::from_string("192.168.0.1");
	PublicConnectionStaticAddress pc(ip_address);
	ASSERT_EQ(ip_address, pc.get_ip_address());
}

TEST(PublicConnectionStaticAddress, manager) {
	InternetServiceProvider isp(std::shared_ptr<IpAddressPool>(new UniqueIpAddressPoolV4(IpAddress::from_string("192.168.0.0").to_v4(), IpAddress::from_string("192.168.1.255").to_v4())));
	PublicConnectionStaticAddress* pc = new PublicConnectionStaticAddress(IpAddress::from_string("192.168.0.1"));
	isp.add_connection(std::unique_ptr<PublicConnection>(pc));
	ASSERT_EQ(&isp, &pc->get_manager());
}

TEST(PublicConnectionStaticAddress, constructor_throws) {
	IpAddress addr;
	// Nawiasy klamrowe zgodnie z poradą w https://stackoverflow.com/questions/6447596/googletest-does-not-accept-temporary-object-in-expect-throw
	ASSERT_THROW(PublicConnectionStaticAddress{ addr }, std::invalid_argument);
}
