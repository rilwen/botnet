#include <gtest/gtest.h>
#include "simbo/internet_service_provider.hpp"
#include "simbo/public_connection_dynamic_address.hpp"
#include "simbo/unique_ip_address_pool.hpp"

using namespace simbo;

// Zobacz https://github.com/google/googletest/blob/master/googletest/docs/primer.md

class PublicConnectionDynamicAddressTest : public ::testing::Test {
protected:
	PublicConnectionDynamicAddressTest() {
		pool = make_unique_ip_address_pool(IpAddress::from_string("192.168.1.1"), IpAddress::from_string("192.168.1.255"));
		creation_time = 0;
		max_lifetime = 60;
	}
	std::shared_ptr<IpAddressPool> pool;
	seconds_t creation_time;
	seconds_t max_lifetime;
};

TEST_F(PublicConnectionDynamicAddressTest, constructor) {
	PublicConnectionDynamicAddress conn(pool, max_lifetime, creation_time);
	ASSERT_FALSE(conn.get_ip_address().is_unspecified());
	ASSERT_EQ(creation_time, conn.valid_from());
	ASSERT_EQ(1, pool->allocated_size());
}

TEST_F(PublicConnectionDynamicAddressTest, manager) {
	InternetServiceProvider isp(pool);
	PublicConnectionDynamicAddress* pc = new PublicConnectionDynamicAddress(pool, max_lifetime, creation_time);
	isp.add_connection(std::unique_ptr<PublicConnection>(pc));
	ASSERT_EQ(&isp, &pc->get_manager());
}

TEST_F(PublicConnectionDynamicAddressTest, constructor_throws) {
	ASSERT_THROW(PublicConnectionDynamicAddress(nullptr, max_lifetime, creation_time), std::invalid_argument);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, 0, creation_time), std::domain_error);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, -120, creation_time), std::domain_error);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, infinite_time(), creation_time), std::domain_error);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, undefined_time(), creation_time), std::domain_error);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, max_lifetime, -infinite_time()), std::domain_error);
	ASSERT_THROW(PublicConnectionDynamicAddress(pool, max_lifetime, undefined_time()), std::domain_error);
}

TEST_F(PublicConnectionDynamicAddressTest, refresh_ip_address_throws) {
	PublicConnectionDynamicAddress conn(pool, max_lifetime, creation_time);
	ASSERT_THROW(conn.refresh_ip_address(creation_time - 10), std::domain_error);
	ASSERT_THROW(conn.refresh_ip_address(creation_time), std::domain_error);
	ASSERT_THROW(conn.refresh_ip_address(undefined_time()), std::domain_error);
}

TEST_F(PublicConnectionDynamicAddressTest, refresh_ip_address) {
	IpAddress reserved = pool->allocate();
	PublicConnectionDynamicAddress conn(pool, max_lifetime, creation_time);
	ASSERT_EQ(2, pool->allocated_size());
	ASSERT_NE(reserved, conn.get_ip_address());
	pool->release(reserved);
	ASSERT_EQ(1, pool->allocated_size());
	conn.refresh_ip_address(creation_time + max_lifetime);
	ASSERT_FALSE(conn.get_ip_address().is_unspecified());
	ASSERT_EQ(creation_time + max_lifetime, conn.valid_from());
	ASSERT_EQ(1, pool->allocated_size());
}
