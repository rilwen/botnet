#include "antivirus_config.hpp"
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {
	void AntivirusConfig::validate() const {
		if (!(infection_attempt_detection_probability >= 0 && infection_attempt_detection_probability <= 1)) {
			throw std::domain_error((boost::locale::format("Infection attempt detection probability invalid: {1}") % infection_attempt_detection_probability).str());
		}
		if (to_seconds(scan_interval) <= 0) {
			throw std::domain_error((boost::locale::format("Scan interval must be positive, is: {1}") % scan_interval).str());
		}
	}

	void from_json(const json& j, AntivirusConfig& config) {
		validate_keys(j, "AntivirusConfig", { "infection_attempt_detection_probability", "update_at_startup", "scan_at_startup", "scan_after_update", "scan_interval" }, {});
		try {
			config.infection_attempt_detection_probability = j["infection_attempt_detection_probability"];
			config.update_at_startup = j["update_at_startup"];
			config.scan_at_startup = j["scan_at_startup"];
			config.scan_after_update = j["scan_after_update"];
			config.scan_interval = j["scan_interval"];
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SoftwareVendorConfig", j.dump(), e);
		}
	}
}