#include <gtest/gtest.h>
#include "simbo/json.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/schedule.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"
#include "simbo/sysadmin_config.hpp"

using namespace simbo;

TEST(SysadminConfig, validate) {
	SysadminConfig cfg;
	std::vector<double> probs({ 0.8, 0.2 });
	std::vector<SysadminConfig::maintenance_trigger_ptr> trigger_processes(2);
	trigger_processes[0] = std::make_shared<PausingJumpMarkovProcess>(0.01, 100);
	trigger_processes[1] = std::make_shared<PausingJumpMarkovProcess>(0.0001, 10000);
	cfg.maintenance_triggers = std::make_shared<SparseDiscreteRandomVariable<SysadminConfig::maintenance_trigger_ptr>>(probs, std::move(trigger_processes));
	ASSERT_NO_THROW(cfg.validate());
}

TEST(SysadminConfig, from_json) {
	json j = "{\"maintenance_triggers\": {"
		"\"weights\": [0.8, 0.2],"
		"\"values\": [{\"base_process\": {\"type\": \"poisson\", \"subtype\": \"simple\", \"params\": {\"lambda\": 0.01}}, \"pause_length\": 100},"
		"{\"base_process\": {\"type\": \"poisson\", \"subtype\": \"simple\", \"params\": {\"lambda\": 0.0001}}, \"pause_length\": 10000}]}}"_json;
	SysadminConfig cfg;
	const DateTime start(Date(2016, 4, 20), TimeDuration(18, 30, 0, 0));
	const Schedule schedule(start);
	from_json(j, schedule, cfg);
	ASSERT_NO_THROW(cfg.validate());
}