#include <gtest/gtest.h>
#include "simbo/enums_json.hpp"
#include "simbo/operating_system_type.hpp"

using namespace simbo;

TEST(OperatingSystemType, to_string) {
	ASSERT_STREQ("ANDROID", to_string(OperatingSystemType::ANDROID));
	ASSERT_STREQ("LINUX", to_string(OperatingSystemType::LINUX));
	ASSERT_STREQ("MACOS", to_string(OperatingSystemType::MACOS));
	ASSERT_STREQ("WINDOWS", to_string(OperatingSystemType::WINDOWS));
	ASSERT_STREQ("OTHER", to_string(OperatingSystemType::OTHER));
}

TEST(OperatingSystemType, from_json) {
	json j = "\"ANDROID\""_json;
	OperatingSystemType ost = j;
	ASSERT_EQ(OperatingSystemType::ANDROID, ost);
}

TEST(OperatingSystemType, to_json) {
	json j = OperatingSystemType::LINUX;
	ASSERT_EQ("\"LINUX\"", j.dump());
}
