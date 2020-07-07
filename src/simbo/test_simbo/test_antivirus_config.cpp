#include <gtest/gtest.h>
#include "simbo/antivirus_config.hpp"
#include "simbo/json.hpp"

using namespace simbo;

TEST(AntivirusConfig, validate) {
	AntivirusConfig cfg;
	cfg.infection_attempt_detection_probability = 0.8;
	cfg.scan_interval = TimeDuration(24, 0, 0, 0);
	cfg.validate();
	cfg.infection_attempt_detection_probability = 2;
	ASSERT_THROW(cfg.validate(), std::domain_error);
	cfg.infection_attempt_detection_probability = -2;
	ASSERT_THROW(cfg.validate(), std::domain_error);
	cfg.scan_interval = TimeDuration();
	ASSERT_THROW(cfg.validate(), std::domain_error);
	cfg.scan_interval = TimeDuration(-24, 0, 0, 0);
	ASSERT_THROW(cfg.validate(), std::domain_error);
}

TEST(AntivirusConfig, from_json) {
	json j = "{\"infection_attempt_detection_probability\":0.9,\"update_at_startup\":true,\"scan_after_update\":false,\"scan_at_startup\":true,\"scan_interval\":{\"hours\":24}}"_json;
	AntivirusConfig cfg = j;
	ASSERT_TRUE(cfg.update_at_startup);
	ASSERT_FALSE(cfg.scan_after_update);
	ASSERT_TRUE(cfg.scan_at_startup);
	ASSERT_EQ(0.9, cfg.infection_attempt_detection_probability);
}