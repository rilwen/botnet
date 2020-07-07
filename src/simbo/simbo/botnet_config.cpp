#include "botnet_config.hpp"
#include "bot_state.hpp"
#include "enums_json.hpp"
#include "exceptions.hpp"
#include "interpolators_json.hpp"
#include "json.hpp"
#include "sparse_discrete_random_variable.hpp"

namespace simbo {
	BotnetConfig::BotnetConfig()
		{	}

	void BotnetConfig::validate() const {
		dropper.validate();
		for (const auto& payload_config : payload.get_ys()) {
			payload_config.validate();
		}
		for (double p : probability_of_reconfiguration.get_ys()) {
			if (!(p >= 0 && p <= 1)) {
				throw std::domain_error((boost::locale::format("Reconfiguration probability must be in [0, 1]: {1}") % p).str());
			}
		}
		for (const auto rv : bot_state_weights.get_ys()) {
			if (!rv) {
				throw std::invalid_argument(boost::locale::translate("Bot state weights random variable is null"));
				for (const BotState bs : rv->get_values()) {
					if (bs == BotState::NONE || bs == BotState::SETTING_UP) {
						throw std::invalid_argument(boost::locale::translate("Invalid bot state assignable to hosts after infection"));
					}
				}
			}
		}		
		if (!std::all_of(infectious_email_sent_per_second_by_command_centre.get_ys().begin(), infectious_email_sent_per_second_by_command_centre.get_ys().end(), [](double x) { return x >= 0; })) {
			throw std::domain_error(boost::locale::translate("Number of infectious email per second sent by command centre must be always non-negative"));
		}
		if (!(email_address_list_size >= 0)) {
			throw std::domain_error((boost::locale::format("Email address list size must be non-negative: {1}") % email_address_list_size).str());
		}
	}

	void from_json(const json& j, BotnetConfig& config) {
		validate_keys(j, "BotnetConfig", { "dropper", "payload", "probability_of_reconfiguration", "bot_state_weights", "infectious_email_sent_per_second_by_command_centre", "email_address_list_size",
			"payload_release_schedule" }, {});
		try {
			config.dropper = j["dropper"];
			config.payload = j["payload"];
			config.probability_of_reconfiguration = j["probability_of_reconfiguration"];
			config.bot_state_weights = j["bot_state_weights"].get<decltype(config.bot_state_weights)>();
			config.infectious_email_sent_per_second_by_command_centre = j["infectious_email_sent_per_second_by_command_centre"];
			config.email_address_list_size = j["email_address_list_size"].get<int>();
			config.payload_release_schedule = j["payload_release_schedule"];
			config.validate();			
		} catch (std::exception& e) {
			throw simbo::DeserialisationError("BotnetConfig", j.dump(), e);
		}
	}
}

//namespace nlohmann {
//	simbo::BotnetConfig adl_serializer<simbo::BotnetConfig>::from_json(const json& j) {
//		simbo::validate_keys(j, "BotnetConfig", { "dropper", "payload", "probability_of_reconfiguration", "bot_state_weights", "infectious_email_sent_per_second_by_command_centre", "email_address_list_size",
//		"payload_release_schedule"}, {});
//		try {
//			simbo::BotnetConfig config(j["payload"]);
//			config.dropper = j["dropper"];
//			config.probability_of_reconfiguration = j["probability_of_reconfiguration"];
//			config.bot_state_weights = j["bot_state_weights"].get<decltype(config.bot_state_weights)>();			
//			config.infectious_email_sent_per_second_by_command_centre = j["infectious_email_sent_per_second_by_command_centre"];
//			config.email_address_list_size = j["email_address_list_size"].get<int>();			
//			config.payload_release_schedule = j["payload_release_schedule"];
//			config.validate();
//			return config;
//		} catch (std::exception& e) {
//			throw simbo::DeserialisationError("BotnetConfig", j.dump(), e);
//		}
//	}
//}