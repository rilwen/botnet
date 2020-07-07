#include <gtest/gtest.h>
#include "simbo/host_config.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"

using namespace simbo;

static HostConfig make_valid_config() {
	HostConfig cfg;
	cfg.operating_system = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
		std::vector<double>({ 0.2, 0.2, 0.6 }),
		std::vector<os_full_spec_t>({
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 7), 1),
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 8), 2),
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 1)
	}));
	cfg.antivirus[std::make_pair(OperatingSystemType::WINDOWS, 10)] = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
		std::vector<double>({ 0.3, 0.3, 0.4 }),
		std::vector<Antivirus::full_spec_t>({
		std::make_pair(std::string("A"), 1),
		std::make_pair(std::string("B"), 1),
		std::make_pair(std::string("C"), 1)
	}));
	return cfg;
}

TEST(HostConfig, validate) {
	HostConfig cfg = make_valid_config();
	cfg.validate();	
}

TEST(HostConfig, validate_throws) {
	HostConfig cfg;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	cfg = make_valid_config();
	cfg.operating_system = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
		std::vector<double>({ 0.5, 0.5 }),
		std::vector<os_full_spec_t>({
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 7), 1),
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 8), 2)		
	}));
	cfg.antivirus[std::make_pair(OperatingSystemType::WINDOWS, 10)] = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
		std::vector<double>({ 0.1 }),
		std::vector<Antivirus::full_spec_t>({
		std::make_pair(std::string("A"), 1)
	}));
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	cfg = make_valid_config();
	cfg.antivirus.clear();
	cfg.antivirus[std::make_pair(OperatingSystemType::WINDOWS, 7)] = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);
}