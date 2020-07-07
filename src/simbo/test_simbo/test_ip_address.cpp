#include <gtest/gtest.h>
#include "simbo/ip_address.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(IpAddress, default_constructor) {
	IpAddress address;
	ASSERT_TRUE(address.is_v4());
	ASSERT_FALSE(address.is_v6());
	ASSERT_TRUE(address.is_unspecified());
}

TEST(IpAddress, from_string) {
	IpAddress address = IpAddress::from_string("192.168.0.1");
	ASSERT_TRUE(address.is_v4());
	ASSERT_FALSE(address.is_v6());
	ASSERT_FALSE(address.is_unspecified());
	address = IpAddress::from_string("2000:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_TRUE(address.is_v6());
	ASSERT_FALSE(address.is_v4());
	ASSERT_FALSE(address.is_unspecified());
}

TEST(IpAddress, to_int64) {
	IpAddress address = IpAddress::from_string("192.168.0.1");
	ASSERT_EQ(3232235521, IpAddress::to_int64(address.to_v4().to_bytes()));
	ASSERT_EQ(3232235521, address.to_int64());
	const std::array<unsigned char, 8> bytes8 = { 1, 0, 0, 0, 0, 0, 0, 0 };
	ASSERT_EQ(72057594037927936, IpAddress::to_int64(bytes8));
}

TEST(IpAddress, to_int64_throws) {
	std::array<unsigned char, 8> bytes8 = { 128, 0, 0, 0, 0, 0, 0, 0 };
	ASSERT_THROW(IpAddress::to_int64(bytes8), std::range_error);
	bytes8[0] = 255;
	ASSERT_THROW(IpAddress::to_int64(bytes8), std::range_error);
	bytes8[0] = 127;
	ASSERT_NO_THROW(IpAddress::to_int64(bytes8));
	std::array<unsigned char, 9> bytes9 = { 1, 0, 0, 0, 0, 0, 0, 0, 0 };
	ASSERT_THROW(IpAddress::to_int64(bytes9), std::range_error);
}

TEST(IpAddress, range_size) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("192.168.0.255");
	ASSERT_EQ(256, IpAddress::range_size(addr1, addr2));
	ASSERT_EQ(1, IpAddress::range_size(addr1, addr1));
	addr1 = IpAddress::from_string("192.168.1.128");
	addr2 = IpAddress::from_string("192.168.3.127");
	ASSERT_EQ(256 + 128 + 128, IpAddress::range_size(addr1, addr2));
	addr1 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0000");
	addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:00ff");
	ASSERT_EQ(256, IpAddress::range_size(addr1, addr2));
}

TEST(IpAddress, range_size_throws) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("192.168.0.255");
	ASSERT_THROW(IpAddress::range_size(addr2, addr1), std::invalid_argument);
	addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(IpAddress::range_size(addr2, addr1), std::invalid_argument);
	ASSERT_THROW(IpAddress::range_size(addr1, addr2), std::invalid_argument);
	addr1 = IpAddress::from_string("2000:0db8:85a3:0000:0000:8a2e:0370:7334");
	assert(addr1 < addr2);
	ASSERT_THROW(IpAddress::range_size(addr1, addr2), std::range_error);
}

TEST(IpAddress, distance) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("192.168.0.255");
	ASSERT_EQ(255, IpAddress::distance(addr1, addr2));
	ASSERT_EQ(255, IpAddress::distance(addr2, addr1));
	ASSERT_EQ(0, IpAddress::distance(addr1, addr1));
	addr1 = IpAddress::from_string("192.168.1.128");
	addr2 = IpAddress::from_string("192.168.3.127");
	ASSERT_EQ(256 + 128 + 127, IpAddress::distance(addr1, addr2));
	ASSERT_EQ(256 + 128 + 127, IpAddress::distance(addr2, addr1));
	addr1 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:0000");
	addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:00ff");
	ASSERT_EQ(255, IpAddress::distance(addr1, addr2));
	ASSERT_EQ(255, IpAddress::distance(addr2, addr1));
}

TEST(IpAddress, distance_throws) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(IpAddress::distance(addr2, addr1), std::invalid_argument);
	ASSERT_THROW(IpAddress::distance(addr1, addr2), std::invalid_argument);
	addr1 = IpAddress::from_string("2000:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(IpAddress::distance(addr1, addr2), std::range_error);
}

TEST(IpAddress, same_type) {
	IpAddress addr1 = IpAddress::from_string("192.168.0.0");
	IpAddress addr2 = IpAddress::from_string("2001:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_FALSE(IpAddress::same_type(addr1, addr2));
	ASSERT_FALSE(IpAddress::same_type(addr2, addr1));
	ASSERT_TRUE(IpAddress::same_type(addr1, addr1));
	ASSERT_TRUE(IpAddress::same_type(addr2, addr2));
	addr2 = IpAddress::from_string("192.168.0.255");
	ASSERT_TRUE(IpAddress::same_type(addr1, addr2));
	ASSERT_TRUE(IpAddress::same_type(addr2, addr1));
}

TEST(IpAddress, from_json) {
	json j = "\"192.168.0.1\""_json;
	IpAddress addr = j;
	ASSERT_EQ(3232235521, IpAddress::to_int64(addr.to_v4().to_bytes()));
}
