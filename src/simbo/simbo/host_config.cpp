#include <unordered_set>
#include "exceptions.hpp"
#include "host_config.hpp"
#include "json.hpp"
#include "random_variable_json.hpp"
#include "sparse_discrete_random_variable.hpp"

namespace simbo {
	void HostConfig::validate() const {
		if (!operating_system) {
			throw std::invalid_argument(boost::locale::translate("Null operating system distribution"));
		}
		std::unordered_set<os_spec_t, Hasher> allowed_operating_systems; // Zbiór dozwolonych systemów operacyjnych.
		for (const os_full_spec_t& full_spec : operating_system->get_values()) {
			allowed_operating_systems.insert(full_spec.first);
		}
		for (const auto& os_av : antivirus) {
			if (!allowed_operating_systems.count(os_av.first)) {
				throw std::invalid_argument((boost::locale::format("Antivirus configuration provided for operating system {1} which is not expected in this HostConfig") % os_av.first).str());
			}
			if (!os_av.second) {
				throw std::invalid_argument((boost::locale::format("Null antivirus configuration for operating system {1}") % os_av.first).str());
			}
		}
	}

	void from_json(const json& j, HostConfig& config) {
		validate_keys(j, "HostConfig", { "operating_system", "antivirus" }, {});
		try {
			config.operating_system = j["operating_system"];
			config.antivirus = j["antivirus"].get<decltype(config.antivirus)>();
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("HostConfig", j.dump(), e);
		}
	}
}