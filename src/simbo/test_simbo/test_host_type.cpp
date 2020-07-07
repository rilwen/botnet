#include <gtest/gtest.h>
#include "simbo/host_type.hpp"

using namespace simbo;

TEST(HostType, is_host_type_fixed) {
	ASSERT_TRUE(is_host_type_fixed(HostType::DESKTOP));
	ASSERT_TRUE(is_host_type_fixed(HostType::SERVER));
	ASSERT_FALSE(is_host_type_fixed(HostType::LAPTOP));
}