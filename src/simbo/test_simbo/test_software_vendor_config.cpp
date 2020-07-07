#include <gtest/gtest.h>
#include "simbo/software_vendor_config.hpp"
#include "simbo/software_vendor_config_json.hpp"

using namespace simbo;

TEST(SoftwareVendorConfig, validate) {
	SoftwareVendorConfig<std::string> cfg;
	cfg.release_schedules["game"] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>(
		std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
		std::vector<int>({ 1, 3 })
	);
	cfg.validate();
	cfg.release_schedules["game"] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>(
		std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
		std::vector<int>({ 1, 1 })
		);
	ASSERT_THROW(cfg.validate(), std::invalid_argument);
	cfg.release_schedules["game"] = MonotonicPiecewiseConstantInterpolator1D<DateTime, int>(
		std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
		std::vector<int>({ 2, 1 })
		);
	ASSERT_THROW(cfg.validate(), std::invalid_argument);
}

TEST(SoftwareVendorConfig, from_json_string_spec) {
	json j = "{\"release_schedules\":{\"A\":{\"xs\":[\"2018-01-01\"],\"ys\":[1]}}}"_json;
	SoftwareVendorConfig<std::string> cfg = j;
	ASSERT_EQ(1, cfg.release_schedules.size());
	ASSERT_TRUE(cfg.release_schedules.count("A"));
	ASSERT_EQ(1, cfg.release_schedules["A"].size());
}

TEST(SoftwareVendorConfig, from_json_int_spec) {
	json j = "{\"release_schedules\":[[101, {\"xs\":[\"2018-01-01\"],\"ys\":[1]}]]}"_json;
	SoftwareVendorConfig<int> cfg = j;
	ASSERT_EQ(1, cfg.release_schedules.size());
	ASSERT_TRUE(cfg.release_schedules.count(101));
	ASSERT_EQ(1, cfg.release_schedules[101].size());
}