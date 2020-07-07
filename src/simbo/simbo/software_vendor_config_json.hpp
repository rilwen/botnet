#pragma once
#include "interpolators_json.hpp"
#include "json.hpp"
#include "software_vendor_config.hpp"

namespace simbo {
	template <class Spec> void from_json(const json& j, SoftwareVendorConfig<Spec>& config) {
		validate_keys(j, "SoftwareVendorConfig", { "release_schedules" }, {});
		try {
			config.release_schedules = j["release_schedules"].get<decltype(config.release_schedules)>();
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("SoftwareVendorConfig", j.dump(), e);
		}
	}
}