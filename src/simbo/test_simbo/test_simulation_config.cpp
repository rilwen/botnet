#include <cmath>
#include <fstream>
#include <gtest/gtest.h>
#include "simbo/constant_random_variable.hpp"
#include "simbo/json.hpp"
#include "simbo/local_network_type.hpp"
#include "simbo/operating_system.hpp"
#include "simbo/pausing_jump_markov_process.hpp"
#include "simbo/schedule.hpp"
#include "simbo/semi_predetermined_process.hpp"
#include "simbo/simple_poisson_process.hpp"
#include "simbo/simulation_config.hpp"
#include "simbo/sparse_discrete_random_variable.hpp"

using namespace simbo;

TEST(SimulationConfig, validate) {
	SimulationConfig cfg;
	auto setup = [&cfg]() {

		cfg = SimulationConfig();
		cfg.random_seed = 3423423;
		
		cfg.public_connections_config.min_ip_address = IpAddress::from_string("192.168.0.1");
		cfg.public_connections_config.max_ip_address = IpAddress::from_string("192.168.0.255");
		cfg.public_connections_config.dynamic_ip_lifetime = 7 * 24 * 3600;
		
		cfg.users_config.laptop_config.operating_system = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
			std::vector<double>({ 0.2, 0.2, 0.6 }),
			std::vector<os_full_spec_t>({
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 7), 1),
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 8), 2),
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 1)
		}));
		cfg.users_config.laptop_config.antivirus[std::make_pair(OperatingSystemType::WINDOWS, 10)] = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
			std::vector<double>({ 1 }), std::vector<Antivirus::full_spec_t>({ Antivirus::full_spec_t("McCoffeeAV", 1) }));
		cfg.users_config.laptop_sysadmin_config.maintenance_triggers = std::make_shared<SparseDiscreteRandomVariable<SysadminConfig::maintenance_trigger_ptr>>(
			std::vector<double>({ 0.7, 0.3 }),
			std::vector<SysadminConfig::maintenance_trigger_ptr>({
			std::make_shared<PausingJumpMarkovProcess>(1e-5, 1e5),
			std::make_shared<PausingJumpMarkovProcess>(1e-4, 1e4)
		}));
		UserConfig user_config;
		user_config.uses_laptop = false;
		user_config.environments = std::vector<UserEnvironmentType>({ UserEnvironmentType::HOME, UserEnvironmentType::OFFICE, UserEnvironmentType::NONE });
		user_config.environment_process = SemiPredeterminedProcess<int>::make_seasonal_weekly(
			std::vector<std::pair<TimeDuration, TimeDuration>>({
			std::make_pair(TimeDuration(0, 0, 0, 0), TimeDuration(9, 0, 0, 0)),
			std::make_pair(TimeDuration(9, 15, 0, 0), TimeDuration(16, 45, 0, 0)),
			std::make_pair(TimeDuration(17, 0, 0, 0), TimeDuration(18, 0, 0, 0)),
			std::make_pair(TimeDuration(19, 0, 0, 0), TimeDuration(21, 0, 0, 0)),
			std::make_pair(TimeDuration(22, 30, 0, 0), TimeDuration(23, 59, 59, 0)),
		}),
		std::vector<int>({2, 1, 2, 0, 2}),
		std::vector<std::pair<TimeDuration, TimeDuration>>({
			std::make_pair(TimeDuration(0, 0, 0, 0), TimeDuration(10, 0, 0, 0)),
			std::make_pair(TimeDuration(11, 00, 0, 0), TimeDuration(20, 0, 0, 0)),
			std::make_pair(TimeDuration(21, 0, 0, 0), TimeDuration(23, 59, 59, 0)),
		}),
		std::vector<int>({2, 0, 2}), DateTime(Date(2018, 5, 1), TimeDuration(0, 0, 0, 0)), Date(2018, 5, 1), Date(2018, 6, 1));
		user_config.initial_email_opening_probability = std::make_shared<ConstantRandomVariable<double>>(0.8);
		user_config.probability_email_activated= std::make_shared<ConstantRandomVariable<double>>(0.3);
		user_config.email_opening_probability_multiplier_exponent
			= std::make_shared<ConstantRandomVariable<double>>(2);
		user_config.probability_instant_email_check = std::make_shared<ConstantRandomVariable<double>>(0.5);
		user_config.personal_email_check_process = std::make_shared<SimplePoissonProcess>(1 / 3600.);
		user_config.work_email_check_process = std::make_shared<SimplePoissonProcess>(1 / 900.);
		user_config.count = 1600;
		cfg.users_config.users["OFFICE_WORKER"] = user_config;

		cfg.email_config.filtering_efficiency["houli"] = 0.99999;
		cfg.email_config.filtering_efficiency["pied_piper"] = 0.23;
		cfg.email_config.personal_email_provider = std::make_shared<SparseDiscreteRandomVariable<std::string>>(
			std::vector<double>({ 0.85, 0.15 }),
			std::vector<std::string>({ "houli", "pied_piper" })
			);
		cfg.email_config.corporate_email_provider = std::make_shared<SparseDiscreteRandomVariable<std::string>>(
			std::vector<double>({ 0.999, 0.001 }),
			std::vector<std::string>({ "houli", "pied_piper" })
			);

		const auto home_os_rv = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
			std::vector<double>({ 0.2, 0.2, 0.55, 0.05 }),
			std::vector<os_full_spec_t>({
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 7), 1),
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 8), 2),
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 1),
			std::make_pair(std::make_pair(OperatingSystemType::LINUX, 204010), 0),
		}));

		const auto desktop_corp_os_rv = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
			std::vector<double>({ 0.9, 0.1 }),
			std::vector<os_full_spec_t>({
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 1),
			std::make_pair(std::make_pair(OperatingSystemType::LINUX, 204010), 0),
		}));

		const auto server_corp_os_rv = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
			std::vector<double>({ 0.5, 0.5 }),
			std::vector<os_full_spec_t>({
			std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 1),
			std::make_pair(std::make_pair(OperatingSystemType::LINUX, 204010), 0),
		}));

		const auto rarely_av = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
			std::vector<double>({ 0.3, 0.7 }),
			std::vector<Antivirus::full_spec_t>({
			Antivirus::full_spec_t("McCoffeeAV", 1),
			Antivirus::full_spec_t("", 1) // Brak antywirusa.
		}));
		const auto usually_av = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
			std::vector<double>({0.8, 0.2}),
			std::vector<Antivirus::full_spec_t>({
			Antivirus::full_spec_t("McCoffeeAV", 1),
			Antivirus::full_spec_t("", 1) // Brak antywirusa.
		}));
		const auto always_av = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
			Antivirus::full_spec_t("McCoffeeAV", 1));
		const auto never_av = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
			Antivirus::full_spec_t("", 1));

		HostConfig::antivirus_cfg_t home_av_cfg;
		home_av_cfg[std::make_pair(OperatingSystemType::WINDOWS, 7)] = rarely_av;
		home_av_cfg[std::make_pair(OperatingSystemType::WINDOWS, 8)] = usually_av;
		home_av_cfg[std::make_pair(OperatingSystemType::WINDOWS, 10)] = usually_av;
		home_av_cfg[std::make_pair(OperatingSystemType::LINUX, 204010)] = never_av;

		HostConfig::antivirus_cfg_t corp_av_cfg;
		corp_av_cfg[std::make_pair(OperatingSystemType::WINDOWS, 10)] = always_av;
		corp_av_cfg[std::make_pair(OperatingSystemType::LINUX, 204010)] = never_av;

		HostConfig home_host_cfg;
		home_host_cfg.operating_system = home_os_rv;
		home_host_cfg.antivirus = home_av_cfg;

		HostConfig desktop_corp_host_cfg;
		desktop_corp_host_cfg.operating_system = desktop_corp_os_rv;
		desktop_corp_host_cfg.antivirus = corp_av_cfg;

		HostConfig server_corp_host_cfg;
		server_corp_host_cfg.operating_system = server_corp_os_rv;
		server_corp_host_cfg.antivirus = corp_av_cfg;

		SysadminConfig home_sysadmin_cfg;
		home_sysadmin_cfg.maintenance_triggers = std::make_shared<SparseDiscreteRandomVariable<SysadminConfig::maintenance_trigger_ptr>>(
			std::vector<double>({ 0.8, 0.2 }),
			std::vector<SysadminConfig::maintenance_trigger_ptr>({
			std::make_shared<PausingJumpMarkovProcess>(1e-5, 1e5),
			std::make_shared<PausingJumpMarkovProcess>(1e-4, 1e4)
		}));
		SysadminConfig corp_sysadmin_cfg;
		corp_sysadmin_cfg.maintenance_triggers = std::make_shared<SparseDiscreteRandomVariable<SysadminConfig::maintenance_trigger_ptr>>(
			std::vector<double>({ 1 }),
			std::vector<SysadminConfig::maintenance_trigger_ptr>({
			std::make_shared<PausingJumpMarkovProcess>(1e-4, 1e4)
		}));

		
		// Zmienna robocza.
		LocalNetworkConfig ln_cfg;
		ln_cfg.country = 1;
		ln_cfg.allowed_users.insert("OFFICE_WORKER");
		ln_cfg.typ = LocalNetworkType::HOUSEHOLD;
		ln_cfg.sysadmin_config = home_sysadmin_cfg;
		ln_cfg.count = 800;
		ln_cfg.fixed_hosts[HostType::DESKTOP] = std::make_pair(home_host_cfg, 1);
		cfg.local_network_configs.push_back(ln_cfg);
		ln_cfg.count = 150;
		ln_cfg.fixed_hosts[HostType::DESKTOP] = std::make_pair(home_host_cfg, 2);
		cfg.local_network_configs.push_back(ln_cfg);
		ln_cfg.count = 50;
		ln_cfg.fixed_hosts[HostType::DESKTOP] = std::make_pair(home_host_cfg, 3);
		cfg.local_network_configs.push_back(ln_cfg);

		ln_cfg.typ = LocalNetworkType::CORPORATE;
		ln_cfg.sysadmin_config = corp_sysadmin_cfg;
		ln_cfg.count = 300;
		ln_cfg.fixed_hosts[HostType::DESKTOP] = std::make_pair(desktop_corp_host_cfg, 5);
		ln_cfg.fixed_hosts[HostType::SERVER] = std::make_pair(server_corp_host_cfg, 1);
		cfg.local_network_configs.push_back(ln_cfg);
		ln_cfg.count = 6;
		ln_cfg.fixed_hosts[HostType::DESKTOP] = std::make_pair(desktop_corp_host_cfg, 50);
		ln_cfg.fixed_hosts[HostType::SERVER] = std::make_pair(server_corp_host_cfg, 1);
		cfg.local_network_configs.push_back(ln_cfg);
		
		cfg.operating_system_vendors_config.vendors["Miko-Sofcik"].release_schedules[std::make_pair(OperatingSystemType::WINDOWS, 7)] = OperatingSystem::vendor_t::config_t::release_schedule_t(
			std::vector<DateTime>({DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
			std::vector<int>({1, 2})
		);
		cfg.operating_system_vendors_config.vendors["Miko-Sofcik"].release_schedules[std::make_pair(OperatingSystemType::WINDOWS, 8)] = OperatingSystem::vendor_t::config_t::release_schedule_t(
			std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
			std::vector<int>({ 2, 3 })
		);
		cfg.operating_system_vendors_config.vendors["Miko-Sofcik"].release_schedules[std::make_pair(OperatingSystemType::WINDOWS, 10)] = OperatingSystem::vendor_t::config_t::release_schedule_t(
			std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
			std::vector<int>({ 1, 2 })
		);
		cfg.operating_system_vendors_config.vendors["Impala"].release_schedules[std::make_pair(OperatingSystemType::LINUX, 204010)] = OperatingSystem::vendor_t::config_t::release_schedule_t(
			std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()) }),
			std::vector<int>({ 0 })
		);

		cfg.antiviruses_config.vendors["McCoffee"].release_schedules["McCoffeeAV"] = Antivirus::vendor_t::config_t::release_schedule_t(
			std::vector<DateTime>({ DateTime(Date(2018, 1, 1), TimeDuration()), DateTime(Date(2018, 6, 1), TimeDuration()) }),
			std::vector<int>({ 1, 2 })
		);
		cfg.antiviruses_config.antiviruses["McCoffeeAV"].update_at_startup = false;
		cfg.antiviruses_config.antiviruses["McCoffeeAV"].infection_attempt_detection_probability = 0.8;
		cfg.antiviruses_config.antiviruses["McCoffeeAV"].scan_interval = TimeDuration(24, 0, 0, 0);
	};
	
	setup();
	cfg.validate();

	setup();
	cfg.random_seed = 0;
	cfg.validate();

	setup();
	cfg.users_config.users["OFFICE_WORKER"].environments = std::vector<UserEnvironmentType>({ UserEnvironmentType::HOME, UserEnvironmentType::NONE });
	cfg.users_config.users["OFFICE_WORKER"].work_email_check_process = nullptr;
	cfg.validate();

	setup();
	cfg.local_network_configs.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.local_network_configs[0].count = -10;
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	std::swap(cfg.public_connections_config.min_ip_address, cfg.public_connections_config.max_ip_address);
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.public_connections_config.max_ip_address = IpAddress::from_string("2000:0db8:85a3:0000:0000:8a2e:0370:7334");
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.public_connections_config.max_ip_address = IpAddress();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.public_connections_config.min_ip_address = IpAddress();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.public_connections_config.dynamic_ip_lifetime = 0;
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	cfg.public_connections_config.dynamic_ip_lifetime = -10;
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	cfg.public_connections_config.dynamic_ip_lifetime = nan("");
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	cfg.local_network_configs[0].fixed_hosts.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.local_network_configs[0].fixed_hosts[HostType::DESKTOP].first.operating_system = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].count = 0;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].count = 10000;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].count = -10;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].count = 10;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.laptop_config.operating_system = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.email_config.filtering_efficiency.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["ROAD_WARRIOR"] = UserConfig();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.email_config.filtering_efficiency.clear();
	cfg.email_config.filtering_efficiency["houli"] = 1.0;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].personal_email_check_process = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.users["OFFICE_WORKER"].environments.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.local_network_configs[0].sysadmin_config.maintenance_triggers = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.laptop_sysadmin_config.maintenance_triggers = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.operating_system_vendors_config.vendors.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.laptop_config.operating_system = nullptr;
	ASSERT_THROW(cfg.validate(), std::invalid_argument);	

	setup();
	cfg.antiviruses_config.antiviruses.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.antiviruses_config.vendors["McCoffee"].release_schedules.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.antiviruses_config.vendors.clear();
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.antiviruses_config.vendors[""] = cfg.antiviruses_config.vendors["McCoffee"];
	cfg.antiviruses_config.vendors.erase("McCoffee");
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.users_config.laptop_config.operating_system = std::make_shared<SparseDiscreteRandomVariable<os_full_spec_t>>(
		std::vector<double>({ 1 }),
		std::vector<os_full_spec_t>({
		std::make_pair(std::make_pair(OperatingSystemType::WINDOWS, 10), 100)
	}));
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	cfg.users_config.laptop_config.antivirus[std::make_pair(OperatingSystemType::WINDOWS, 10)] = std::make_shared<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>(
		std::vector<double>({ 1 }), std::vector<Antivirus::full_spec_t>({ Antivirus::full_spec_t("McCoffeeAV", 10) }));
	ASSERT_THROW(cfg.validate(), std::domain_error);

	setup();
	cfg.local_network_configs[0].allowed_users.insert("");
	ASSERT_THROW(cfg.validate(), std::invalid_argument);

	setup();
	cfg.local_network_configs[0].country = -1;
	ASSERT_THROW(cfg.validate(), std::domain_error);
	setup();
	cfg.local_network_configs[0].country = 256;
	ASSERT_THROW(cfg.validate(), std::domain_error);
}

//TEST(SimulationConfig, to_json) {
//	SimulationConfig cfg;
//	cfg.random_seed = 3423423;
//	cfg.number_local_networks = 1000;
//	cfg.local_network_type_weights[LocalNetworkType::CORPORATE] = 1;
//	cfg.local_network_type_weights[LocalNetworkType::HOUSEHOLD] = 11;
//	cfg.local_network_type_weights[LocalNetworkType::PUBLIC] = 3;
//	json j = cfg;
//	ASSERT_EQ("{\"local_network_type_weights\":[[\"CORPORATE\",1.0],[\"HOUSEHOLD\",11.0],[\"PUBLIC\",3.0]],\"number_local_networks\":1000,\"random_seed\":3423423}", j.dump());
//}

TEST(SimulationConfig, from_json) {
	std::ifstream json_file("test_simbo/resources/test_simulation_config_from_json.json");
	json j;
	json_file >> j;
	Schedule schedule(DateTime(Date(1990, 1, 1), TimeDuration(0, 0, 0, 0)), TimeDuration(24, 0, 0, 0), 8);
	SimulationConfig cfg;
	from_json(j, schedule, cfg);
	cfg.validate();
	ASSERT_EQ(3423423, cfg.random_seed);
	ASSERT_EQ(4, cfg.local_network_configs.size());
	ASSERT_EQ(750, cfg.local_network_configs[0].count);
	ASSERT_EQ(1, cfg.users_config.users.size());
	ASSERT_EQ(1, cfg.users_config.laptop_config.operating_system->get_number_values());
}
