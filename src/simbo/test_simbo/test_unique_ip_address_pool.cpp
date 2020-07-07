#include <gtest/gtest.h>
#include "simbo/unique_ip_address_pool.hpp"

using namespace simbo;

TEST(UniqueIpAddressPool, make_unique_ip_address_pool_throws) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(make_unique_ip_address_pool(addr1, addr2), std::invalid_argument);
	ASSERT_THROW(make_unique_ip_address_pool(addr2, addr1), std::invalid_argument);

	addr2 = IpAddress::from_string("192.168.0.255");
	ASSERT_THROW(make_unique_ip_address_pool(addr2, addr1), std::invalid_argument);

	addr1 = IpAddress::from_string("0000:0db8:85a3:0000:0000:8a2e:0370:7334");
	addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(make_unique_ip_address_pool(addr1, addr2), std::range_error);
}

TEST(UniqueIpAddressPool, make_unique_ip_address_pool) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("192.168.0.248");
	auto pool = make_unique_ip_address_pool(addr1, addr2);
	ASSERT_NE(nullptr, pool);
	ASSERT_EQ(addr1, pool->min());
	ASSERT_EQ(addr2, pool->max());
	ASSERT_EQ(249, pool->total_size());
	ASSERT_EQ(0, pool->allocated_size());
	addr1 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0000");
	addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:01ff");
	pool = make_unique_ip_address_pool(addr1, addr2);
	ASSERT_NE(nullptr, pool);
	ASSERT_EQ(addr1, pool->min());
	ASSERT_EQ(addr2, pool->max());
	ASSERT_EQ(512, pool->total_size());
	ASSERT_EQ(0, pool->allocated_size());
}

TEST(UniqueIpAddressPoolV4, constructor) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("192.168.0.248");
	UniqueIpAddressPoolV4 pool(addr1.to_v4(), addr2.to_v4());
	ASSERT_EQ(addr1, pool.min());
	ASSERT_EQ(addr2, pool.max());
	ASSERT_EQ(249, pool.total_size());
	ASSERT_EQ(0, pool.allocated_size());
}

TEST(UniqueIpAddressPoolV4, test) {
	const IpAddress min_addr = IpAddress::from_string("192.168.0.1");
	const IpAddress addr = IpAddress::from_string("192.168.0.2");
	const IpAddress max_addr = IpAddress::from_string("192.168.0.3");
	UniqueIpAddressPoolV4 pool(min_addr.to_v4(), max_addr.to_v4());
	ASSERT_EQ(min_addr, pool.allocate());
	ASSERT_EQ(1, pool.allocated_size());
	ASSERT_EQ(addr, pool.allocate());
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(max_addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	ASSERT_THROW(pool.allocate(), std::logic_error);
	pool.release(max_addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(max_addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	pool.release(addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	ASSERT_EQ(min_addr, pool.update(min_addr));
	ASSERT_EQ(3, pool.allocated_size());
	pool.release(min_addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(min_addr, pool.update(max_addr));
	ASSERT_EQ(2, pool.allocated_size());
}

TEST(UniqueIpAddressPoolV6, constructor) {
	IpAddress addr1 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0000");
	IpAddress addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:01ff");
	UniqueIpAddressPoolV6 pool(addr1.to_v6(), addr2.to_v6());
	ASSERT_EQ(addr1, pool.min());
	ASSERT_EQ(addr2, pool.max());
	ASSERT_EQ(512, pool.total_size());
	ASSERT_EQ(0, pool.allocated_size());
}

TEST(UniqueIpAddressPoolV6, test) {
	const IpAddress min_addr = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0000");
	const IpAddress addr = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0001");
	const IpAddress max_addr = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0002");
	UniqueIpAddressPoolV6 pool(min_addr.to_v6(), max_addr.to_v6());
	ASSERT_EQ(min_addr, pool.allocate());
	ASSERT_EQ(1, pool.allocated_size());
	ASSERT_EQ(addr, pool.allocate());
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(max_addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	ASSERT_THROW(pool.allocate(), std::logic_error);
	pool.release(max_addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(max_addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	pool.release(addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(addr, pool.allocate());
	ASSERT_EQ(3, pool.allocated_size());
	ASSERT_EQ(min_addr, pool.update(min_addr));
	ASSERT_EQ(3, pool.allocated_size());
	pool.release(min_addr);
	ASSERT_EQ(2, pool.allocated_size());
	ASSERT_EQ(min_addr, pool.update(max_addr));
	ASSERT_EQ(2, pool.allocated_size());
}
