#include "botnet_payload_config.hpp"
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {
	BotnetPayloadConfig::BotnetPayloadConfig()
	{}

	void BotnetPayloadConfig::validate() const {
		if (!(infectious_emails_sent_per_second_per_bot >= 0)) {
			throw std::domain_error((boost::locale::format("Number of infectious email per second per bot must be non-negative: {1}") % infectious_emails_sent_per_second_per_bot).str());
		}
	}

	void from_json(const json& j, BotnetPayloadConfig& config) {
		simbo::validate_keys(j, "BotnetPayloadConfig", { "ping_interval", "ping_at_start", "reconfiguration_interval", "reconfigure_at_start", "infectious_emails_sent_per_second_per_bot", "min_detecting_av_release" }, {});
		try {
			config.ping_interval = j["ping_interval"];
			config.reconfiguration_interval = j["reconfiguration_interval"];
			config.ping_at_start = j["ping_at_start"];
			config.reconfigure_at_start = j["reconfigure_at_start"];
			config.infectious_emails_sent_per_second_per_bot = j["infectious_emails_sent_per_second_per_bot"];
			config.min_detecting_av_release = j["min_detecting_av_release"].get<decltype(config.min_detecting_av_release)>();
			config.validate();
		} catch (std::exception& e) {
			throw simbo::DeserialisationError("BotnetPayloadConfig", j.dump(), e);
		}
	}
}

//namespace nlohmann {
//	simbo::BotnetPayloadConfig adl_serializer<simbo::BotnetPayloadConfig>::from_json(const json& j) {
//		simbo::validate_keys(j, "BotnetPayloadConfig", { "ping_interval", "ping_at_start", "reconfiguration_interval", "reconfigure_at_start", "infectious_emails_sent_per_second_per_bot", "min_detecting_av_release" }, {});
//		try {
//			const simbo::ActionInterval ping_interval = j["ping_interval"];
//			const simbo::ActionInterval reconfiguration_interval = j["reconfiguration_interval"];
//			simbo::BotnetPayloadConfig config(ping_interval, reconfiguration_interval);
//			config.ping_at_start = j["ping_at_start"];
//			config.reconfigure_at_start = j["reconfigure_at_start"];
//			config.infectious_emails_sent_per_second_per_bot = j["infectious_emails_sent_per_second_per_bot"];
//			config.min_detecting_av_release = j["min_detecting_av_release"].get<decltype(config.min_detecting_av_release)>();
//			config.validate();
//			return config;
//		} catch (std::exception& e) {
//			throw simbo::DeserialisationError("BotnetPayloadConfig", j.dump(), e);
//		}
//	}
//}