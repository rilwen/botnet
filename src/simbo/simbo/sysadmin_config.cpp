#include "exceptions.hpp"
#include "json.hpp"
#include "pausing_jump_markov_process.hpp"
#include "sparse_discrete_random_variable.hpp"
#include "sysadmin_config.hpp"

namespace simbo {
	void SysadminConfig::validate() const {
		if (!maintenance_triggers) {
			throw std::invalid_argument(boost::locale::translate("Null maintenance trigger process RV"));
		}
		for (const auto maintenance_trigger : maintenance_triggers->get_values()) {
			if (!maintenance_trigger) {
				throw std::invalid_argument(boost::locale::translate("Null maintenance trigger process"));
			}
		}
	}

	void from_json(const json& j, const Schedule& schedule, SysadminConfig& cfg) {
		validate_keys(j, "SysadminConfig", { "maintenance_triggers" }, {});
		try {
			std::unique_ptr<SparseDiscreteRandomVariable<SysadminConfig::maintenance_trigger_ptr>> mt;
			from_json(j["maintenance_triggers"], schedule, mt);
			cfg.maintenance_triggers = std::move(mt);
		} catch (const std::exception& e) {
			throw DeserialisationError("SysadminConfig", j.dump(), e);
		}
	}
}