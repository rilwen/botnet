#include <gtest/gtest.h>
#include "simbo/ip_address_pool.hpp"

namespace simbo {
	TEST(IpAddressPoolTest, advance) {
		std::array<unsigned char, 4> bytes = { 192, 168, 1, 44 };
		
		IpAddressPool::advance(bytes, 0);
		std::array<unsigned char, 4> expected = { 192, 168, 1, 44 };
		ASSERT_EQ(expected, bytes);

		IpAddressPool::advance(bytes, 1);
		expected = { 192, 168, 1, 45 };
		ASSERT_EQ(expected, bytes);
		
		IpAddressPool::advance(bytes, 256);
		expected = { 192, 168, 2, 45 };
		ASSERT_EQ(expected, bytes);

		IpAddressPool::advance(bytes, 255);
		expected = { 192, 168, 3, 44 };
		ASSERT_EQ(expected, bytes);

		IpAddressPool::advance(bytes, 256 * 256 + 1);
		expected = { 192, 169, 3, 45 };
		ASSERT_EQ(expected, bytes);

		bytes = { 255, 255, 255, 100 };
		ASSERT_THROW(IpAddressPool::advance(bytes, 256), std::domain_error);
	}
}
