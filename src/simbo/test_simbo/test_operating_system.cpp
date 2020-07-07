#include <gtest/gtest.h>
#include "simbo/operating_system.hpp"
#include "test_operating_system.h"

using namespace simbo;

TEST(OperatingSystem, build) {
	OperatingSystem::vendor_t::config_t config;
	const auto spec_win = std::make_pair(OperatingSystemType::WINDOWS, 7);
	const auto spec_lin = std::make_pair(OperatingSystemType::LINUX, 263);
	const auto spec_mac = std::make_pair(OperatingSystemType::MACOS, 12);
	config.release_schedules[spec_win] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>();
	config.release_schedules[spec_lin] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>();
	config.release_schedules[spec_mac] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>();
	std::shared_ptr<OperatingSystem::vendor_t> vendor(new OperatingSystem::vendor_t("Vendie", config));
	auto os = OperatingSystem::build(vendor, spec_win, 1);
	ASSERT_NE(nullptr, os);
	ASSERT_EQ(OperatingSystemType::WINDOWS, os->get_type());
	ASSERT_EQ(7, os->get_version_number());
	ASSERT_EQ(1, os->get_release_number());
	os = OperatingSystem::build(vendor, spec_lin, 2);
	ASSERT_NE(nullptr, os);
	ASSERT_EQ(OperatingSystemType::LINUX, os->get_type());
	ASSERT_EQ(263, os->get_version_number());
	ASSERT_EQ(2, os->get_release_number());
	os = OperatingSystem::build(vendor, spec_mac, 3);
	ASSERT_NE(nullptr, os);
	ASSERT_EQ(OperatingSystemType::MACOS, os->get_type());
	ASSERT_EQ(12, os->get_version_number());
	ASSERT_EQ(3, os->get_release_number());
}
