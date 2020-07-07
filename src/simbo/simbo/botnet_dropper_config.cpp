#include "botnet_dropper_config.hpp"
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {

	BotnetDropperConfig::BotnetDropperConfig()
		: setting_up_period(-1) {}

	void BotnetDropperConfig::validate() const {
		if (!(setting_up_period > 0)) {
			throw std::domain_error((boost::locale::format("Setting up period must be positive: {1}") % setting_up_period).str());
		}
	}

	void from_json(const json& j, BotnetDropperConfig& config) {
		validate_keys(j, "BotnetDropperConfig", { "setting_up_period", "max_infected_release" }, {});
		try {
			config.setting_up_period = j["setting_up_period"];
			config.max_infected_release = j["max_infected_release"].get<decltype(config.max_infected_release)>();
			config.validate();			
		} catch (std::exception& e) {
			throw DeserialisationError("BotnetDropperConfig", j.dump(), e);
		}
	}
}

//namespace nlohmann {
//	simbo::BotnetDropperConfig adl_serializer<simbo::BotnetDropperConfig>::from_json(const json& j) {
//		simbo::validate_keys(j, "BotnetDropperConfig", { "setting_up_period", "max_infected_release" }, {});
//		try {
//			simbo::BotnetDropperConfig config;
//			config.setting_up_period = j["setting_up_period"];
//			config.max_infected_release = j["max_infected_release"].get<decltype(config.max_infected_release)>();			
//			config.validate();
//			return config;
//		} catch (std::exception& e) {
//			throw simbo::DeserialisationError("BotnetDropperConfig", j.dump(), e);
//		}
//	}
//}