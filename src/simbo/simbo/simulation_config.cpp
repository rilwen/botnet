#include <algorithm>
#include <unordered_set>
#include "enums_json.hpp"
#include "exceptions.hpp"
#include "json.hpp"
#include "local_network_type.hpp"
#include "operating_system_type.hpp"
#include "simulation_config.hpp"
#include "software_vendor_config_json.hpp"
#include "sparse_discrete_random_variable.hpp"
#include "sysadmin_config.hpp"

namespace simbo {

	template <class Spec, class VendorsConfig, class F> static void validate_vendors(const SparseDiscreteRandomVariable<Spec>& full_specs, const VendorsConfig& vendors_config, F predicate_for_null, const char* category) {
		for (const auto& full_spec : full_specs.get_values()) {
			const auto spec = full_spec.first;
			if (predicate_for_null(spec)) {
				// Specyfikacja braku programu.
				continue;
			}
			bool has_vendor = false;
			for (const auto& kv_vendors : vendors_config) {
				const auto release_schedule_it = kv_vendors.second.release_schedules.find(spec);
				if (release_schedule_it != kv_vendors.second.release_schedules.end()) {
					has_vendor = true;
					const auto max_release_number = release_schedule_it->second.get_ys().back();
					if (full_spec.second > max_release_number) {
						throw std::domain_error((boost::locale::format("Release number for {1} {2} is larger than the final one in vendor's schedule: {3} > {4}") % category % spec % full_spec.second % max_release_number).str());
					}
					break;
				}
			}
			if (!has_vendor) {
				throw std::invalid_argument((boost::locale::format("No vendor for {1} {2}") % category % spec).str());
			}
		}
	}	

	void SimulationConfig::validate() const {
		public_connections_config.validate();
		email_config.validate();
		users_config.validate();		
		operating_system_vendors_config.validate();
		antiviruses_config.validate();
		int total_number_corporate_fixed_hosts = 0;
		int total_number_household_fixed_hosts = 0;
		if (local_network_configs.empty()) {
			throw std::invalid_argument(boost::locale::translate("No local network configurations"));
		}
		for (const LocalNetworkConfig& ln_cfg: local_network_configs) {
			ln_cfg.validate();
			const LocalNetworkType ln_type = ln_cfg.typ;
			int ln_size = 0;
			for (const auto& host_type_and_cfg : ln_cfg.fixed_hosts) {
				const std::pair<HostConfig, int>& host_cfg_and_count = host_type_and_cfg.second;
				const HostConfig& host_cfg = host_cfg_and_count.first;
				ln_size += host_cfg_and_count.second;
				validate_vendors(*host_cfg.operating_system, operating_system_vendors_config.vendors, OperatingSystem::is_null, "operating system");
				for (const auto& av_kv : host_cfg.antivirus) {
					validate_vendors(*av_kv.second, antiviruses_config.vendors, Antivirus::is_null, "antivirus");
				}
			}
			const int nbr_hosts = ln_cfg.count * ln_size;
			if (ln_type == LocalNetworkType::CORPORATE) {
				total_number_corporate_fixed_hosts += nbr_hosts;
			} else if (ln_type == LocalNetworkType::HOUSEHOLD) {
				total_number_household_fixed_hosts += nbr_hosts;
			}
		}
		int total_number_home_fixed_host_users = 0;
		int total_number_office_fixed_host_users = 0;
		for (const auto& user_type_and_cfg : users_config.users) {
			const UserConfig& user_cfg = user_type_and_cfg.second;			
			if (!user_cfg.uses_laptop) {
				const int nbr_users = user_cfg.count;
				if (
					std::find(user_cfg.environments.begin(), user_cfg.environments.end(), UserEnvironmentType::HOME) != user_cfg.environments.end()
					) {
					total_number_home_fixed_host_users += nbr_users;
				}
				if (std::find(user_cfg.environments.begin(), user_cfg.environments.end(), UserEnvironmentType::OFFICE) != user_cfg.environments.end()) {
					total_number_office_fixed_host_users += nbr_users;
				}
			}
		}
		if (total_number_home_fixed_host_users < total_number_household_fixed_hosts) {
			throw std::invalid_argument((boost::locale::format("Number of home users {1} is smaller than number of household fixed hosts {2}") % total_number_home_fixed_host_users % total_number_household_fixed_hosts).str());
		}
		if (total_number_office_fixed_host_users > total_number_corporate_fixed_hosts) {
			throw std::invalid_argument((boost::locale::format("Number of office users {1} is larger than number of corporate fixed hosts {2}") % total_number_office_fixed_host_users % total_number_corporate_fixed_hosts).str());
		}
		validate_vendors(*users_config.laptop_config.operating_system, operating_system_vendors_config.vendors, OperatingSystem::is_null, "operating system");
		for (const auto& av_kv : users_config.laptop_config.antivirus) {
			validate_vendors(*av_kv.second, antiviruses_config.vendors, Antivirus::is_null, "antivirus");
		}
	}

	void SimulationConfig::PublicConnectionsConfig::validate() const {
		if (min_ip_address.is_unspecified()) {
			throw std::invalid_argument(boost::locale::translate("Minimum IP address unspecified"));
		}
		if (max_ip_address.is_unspecified()) {
			throw std::invalid_argument(boost::locale::translate("Maximum IP address unspecified"));
		}
		if (!IpAddress::same_type(min_ip_address, max_ip_address)) {
			throw std::invalid_argument((boost::locale::format("Minimum IP address {1} should be of the same type as maximum IP address {2}") % min_ip_address % max_ip_address).str());
		}
		if (!(min_ip_address <= max_ip_address)) {
			throw std::invalid_argument((boost::locale::format("Minimum IP address {1} should not be greater than maximum IP address {2}") % min_ip_address % max_ip_address).str());
		}
		if (!(dynamic_ip_lifetime > 0)) {
			throw std::domain_error((boost::locale::format("Dynamic IP lifetime should be positive, is {1}") % dynamic_ip_lifetime).str());
		}
	}

	void SimulationConfig::UsersConfig::validate() const {		
		if (users.empty()) {
			throw std::invalid_argument(boost::locale::translate("No user configurations"));
		}
		for (const auto& user_type_and_cfg : users) {
			try {
				user_type_and_cfg.second.validate();
			} catch (const std::exception& e) {
				throw std::invalid_argument((boost::locale::format("Invalid configuration for user type {1}: {2}") % user_type_and_cfg.first % e.what()).str());
			}
		}
		laptop_config.validate();
		laptop_sysadmin_config.validate();
	}

	void SimulationConfig::EmailConfig::validate() const {
		if (filtering_efficiency.empty()) {
			throw std::invalid_argument(boost::locale::translate("No email providers provided"));
		}
		for (const auto& kv : filtering_efficiency) {
			const double eff = kv.second;
			if (!(eff >= 0 && eff <= 1)) {
				throw std::domain_error((boost::locale::format("Email filtering efficiency outside [0, 1] range for provider \"{1}\": {2}") % kv.first % eff).str());
			}
		}
		if (!personal_email_provider) {
			throw std::invalid_argument(boost::locale::translate("Null personal email provider distribution"));
		}
		for (const auto& provider_id : personal_email_provider->get_values()) {
			if (!filtering_efficiency.count(provider_id)) {
				throw std::invalid_argument((boost::locale::format("Unknown personal email provider: {1}") % provider_id).str());
			}
		}
		if (!corporate_email_provider) {
			throw std::invalid_argument(boost::locale::translate("Null corporate email provider distribution"));
		}
		for (const auto& provider_id : corporate_email_provider->get_values()) {
			if (!filtering_efficiency.count(provider_id)) {
				throw std::invalid_argument((boost::locale::format("Unknown corporate email provider: {1}") % provider_id).str());
			}
		}
	}

	void SimulationConfig::OperatingSystemVendorsConfig::validate() const {
		for (const auto& kv1: vendors) {
			const auto& vendor1 = kv1.second;
			vendor1.validate();
			for (const auto& vendor_kv : vendor1.release_schedules) {
				const auto type_version = vendor_kv.first;
				for (const auto& kv2 : vendors) {
					const auto& vendor2 = kv2.second;
					// Sprawdź czy dwóch różnych dostawców nie obsługuje tej samej pary (system operacyjny, wersja).
					if (&kv1 != &kv2 && vendor2.release_schedules.find(type_version) != vendor2.release_schedules.end()) {
						throw std::invalid_argument((boost::locale::format("Vendors \"{1}\" and \"{2}\" both support operating system type {3}, version {4}") % kv1.first % kv2.first % type_version.first % type_version.second).str());
					}
				}
			}
		}
	}

	void SimulationConfig::AntivirusesConfig::validate() const {
		std::unordered_set<std::string> avs_with_vendors;
		for (const auto& kv1 : vendors) {
			if (kv1.first.empty()) {
				throw std::invalid_argument(boost::locale::translate("Empty antivirus vendor name"));
			}
			const auto& vendor1 = kv1.second;
			vendor1.validate();
			for (const auto& vendor_kv : vendor1.release_schedules) {
				const std::string& spec = vendor_kv.first;				
				if (spec.empty()) {
					throw std::invalid_argument((boost::locale::format("Empty antivirus name in antivirus vendor {1} config") % kv1.first).str());
				}
				if (!antiviruses.count(spec)) {
					throw std::invalid_argument((boost::locale::format("Vendor {1} supports antivirus {2} without a configuration") % kv1.first % spec).str());
				}
				avs_with_vendors.insert(spec);
				for (const auto& kv2 : vendors) {
					const auto& vendor2 = kv2.second;
					// Sprawdź czy dwóch różnych dostawców nie obsługuje tej samej pary (system operacyjny, wersja).
					if (&kv1 != &kv2 && vendor2.release_schedules.find(spec) != vendor2.release_schedules.end()) {
						throw std::invalid_argument((boost::locale::format("Vendors {1} and {2} both support antivirus {3}") % kv1.first % kv2.first % spec).str());
					}
				}
			}
		}
		for (const auto& kv : antiviruses) {
			if (kv.first.empty()) {
				throw std::invalid_argument(boost::locale::translate("Empty antivirus name"));
			}
			if (!avs_with_vendors.count(kv.first)) {
				throw std::invalid_argument((boost::locale::format("Antivirus {1} without a vendor") % kv.first).str());
			}
			kv.second.validate();
		}
	}

	static void from_json(const json& j, SimulationConfig::PublicConnectionsConfig& config) {
		validate_keys(j, "SimulationConfig::PublicConnectionsConfig", { "min_ip_address", "max_ip_address", "dynamic_ip_lifetime" }, { });
		try {
			config.min_ip_address = j["min_ip_address"].get<IpAddress>();
			config.max_ip_address = j["max_ip_address"].get<IpAddress>();
			config.dynamic_ip_lifetime = j["dynamic_ip_lifetime"];
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig::PublicConnectionsConfig", j.dump(), e);
		}
	}

	static void from_json(const json& j, SimulationConfig::EmailConfig& config) {
		validate_keys(j, "SimulationConfig::EmailConfig", { "filtering_efficiency", "personal_email_provider", "corporate_email_provider" }, {});
		try {
			config.filtering_efficiency = j["filtering_efficiency"].get<decltype(config.filtering_efficiency)>();
			config.corporate_email_provider = j["corporate_email_provider"];
			config.personal_email_provider = j["personal_email_provider"];
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig::EmailConfig", j.dump(), e);
		}
	}

	static void from_json(const json& j, const Schedule& schedule, SimulationConfig::UsersConfig& config) {
		validate_keys(j, "SimulationConfig::UsersConfig", { "laptop_config", "laptop_sysadmin_config", "users" }, {});
		try {
			config.laptop_config = j["laptop_config"];			
			from_json(j["laptop_sysadmin_config"], schedule, config.laptop_sysadmin_config);
			get_map_from_object(j["users"], "SimulationConfig::UsersConfig", schedule, config.users);
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig::UsersConfig", j.dump(), e);
		}
	}

	static void from_json(const json& j, SimulationConfig::OperatingSystemVendorsConfig& config) {
		validate_keys(j, "SimulationConfig::OperatingSystemVendorsConfig", { "vendors" }, {});
		try {
			get_map_from_object(j["vendors"], "SimulationConfig::OperatingSystemVendorsConfig", config.vendors);
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig::OperatingSystemVendorsConfig", j.dump(), e);
		}
	}

	static void from_json(const json& j, SimulationConfig::AntivirusesConfig& config) {
		validate_keys(j, "SimulationConfig::AntivirusesConfig", { "vendors", "antiviruses" }, {});
		try {
			get_map_from_object(j["vendors"], "SimulationConfig::AntivirusesConfig", config.vendors);
			get_map_from_object(j["antiviruses"], "SimulationConfig::AntivirusesConfig", config.antiviruses);
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig::AntivirusesConfig", j.dump(), e);
		}
	}

	void from_json(const json& j, const Schedule& schedule, SimulationConfig& config) {
		validate_keys(j, "SimulationConfig", { 
			"local_network_configs",
			"public_connections_config",
			"email_config",
			"users_config",
			"operating_system_vendors_config",
			"antiviruses_config"
		}, { "random_seed" });
		try {
			config.random_seed = get_value<uint64_t>(j, "random_seed", 0);
			from_json(j["local_network_configs"], schedule, config.local_network_configs);
			config.public_connections_config = j["public_connections_config"];
			config.email_config = j["email_config"];
			from_json(j["users_config"], schedule, config.users_config);
			config.operating_system_vendors_config = j["operating_system_vendors_config"];
			config.antiviruses_config = j["antiviruses_config"];
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SimulationConfig", j.dump(), e);
		}
	}	
}
