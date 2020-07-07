#include "exceptions.hpp"
#include "enums_json.hpp"
#include "json.hpp"
#include "local_network_config.hpp"

namespace simbo {
	void LocalNetworkConfig::validate() const {
		if (count <= 0) {
			throw std::domain_error((boost::locale::format("Number of local networks must be positive, is {1}") % count).str());
		}
		if (fixed_hosts.empty()) {
			throw std::invalid_argument(boost::locale::translate("No hosts configured"));
		}
		for (const auto& host_type_and_cfg : fixed_hosts) {
			const HostType host_type = host_type_and_cfg.first;
			if (!is_host_type_fixed(host_type)) {
				throw std::invalid_argument((boost::locale::format("Not a fixed host type: {1}") % host_type).str());
			}
			// Sprawdź konfigurację.
			host_type_and_cfg.second.first.validate();
			const int host_count = host_type_and_cfg.second.second;
			if (host_count <= 0) {
				throw std::domain_error((boost::locale::format("Number of hosts of type {1} must be positive, is {2}") % host_type % host_count).str());
			}			
		}
		for (const std::string& allowed_user : allowed_users) {
			if (allowed_user.empty()) {
				throw std::invalid_argument(boost::locale::translate("Empty user configuration name"));
			}
		}
		if (country <= 0 || country >= 256) {
			throw std::domain_error((boost::locale::format("Country code must be in [1, 255] range, is {1}") % country).str());
		}
		sysadmin_config.validate();
	}

	void from_json(const json& j, const Schedule& schedule, LocalNetworkConfig& config) {
		validate_keys(j, "LocalNetworkConfig", { "count", "fixed_hosts", "sysadmin_config", "typ", "allowed_users", "country" }, {});
		try {
			config.count = j["count"];
			config.fixed_hosts = j["fixed_hosts"].get<decltype(config.fixed_hosts)>();
			from_json(j["sysadmin_config"], schedule, config.sysadmin_config);
			config.typ = j["typ"];
			config.allowed_users = j["allowed_users"].get<decltype(config.allowed_users)>();
			config.country = j["country"];
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("LocalNetworkConfig", j.dump(), e);
		}
	}
}