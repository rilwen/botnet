#include "json.hpp"
#include "markov_process_json.hpp"
#include "random_variable_json.hpp"
#include "user_config.hpp"

namespace simbo {
	void UserConfig::validate() const {
		if (count <= 0) {
			throw std::domain_error((boost::locale::format("Negative or zero number of users: {1}") % count).str());
		}
		if (environments.empty()) {
			throw std::invalid_argument(boost::locale::translate("No environments defined"));
		}
		for (size_t i = 0; i < environments.size(); ++i) {
			for (size_t j = 0; j < i; ++j) {
				if (environments[i] == environments[j]) {
					throw std::invalid_argument((boost::locale::format("Elements {1} and {2} of environments vector have the same value: {3}") % i % j % environments[i]).str());
				}
			}
		}
		for (const UserEnvironmentType ue_typ : work_email_environments) {
			if (ue_typ == UserEnvironmentType::NONE) {
				throw std::invalid_argument(boost::locale::translate("Work email environment cannot be NONE"));
			}
			if (std::find(environments.begin(), environments.end(), ue_typ) == environments.end()) {
				throw std::invalid_argument((boost::locale::format("Work email environment {1} not present in the environments vector") % ue_typ).str());
			}
		}
		for (const UserEnvironmentType ue_typ : personal_email_environments) {
			if (ue_typ == UserEnvironmentType::NONE) {
				throw std::invalid_argument(boost::locale::translate("Personal email environment cannot be NONE"));
			}
			if (std::find(environments.begin(), environments.end(), ue_typ) == environments.end()) {
				throw std::invalid_argument((boost::locale::format("Personal email environment {1} not present in the environments vector") % ue_typ).str());
			}
		}
		if (!environment_process) {
			throw std::invalid_argument(boost::locale::translate("Null environment process"));
		}
		if (!probability_email_activated) {
			throw std::invalid_argument(boost::locale::translate("Null distribution of probability of infected email activation"));
		}
		if (!initial_email_opening_probability) {
			throw std::invalid_argument(boost::locale::translate("Null distribution of initial probability of infected email opening"));
		}
		if (!email_opening_probability_multiplier_exponent) {
			throw std::invalid_argument(boost::locale::translate("Null distribution of the exponent of the multiplier of probability of infected email opening"));
		}
		if (!probability_instant_email_check) {
			throw std::invalid_argument(boost::locale::translate("Null distribution of the probability of instantenous email checks"));
		}
		if (!personal_email_check_process) {
			throw std::invalid_argument(boost::locale::translate("Null personal email check process"));
		}
		if (std::find(environments.begin(), environments.end(), UserEnvironmentType::OFFICE) != environments.end()
			&& !work_email_check_process) {
			throw std::invalid_argument(boost::locale::translate("Null work email check process for user working on the OFFICE environment"));
		}
	}

	void from_json(const json& j, const Schedule& schedule, UserConfig& config) {
		validate_keys(j, "UserConfig", { "environments", "environment_process", "uses_laptop", "probability_email_activated", "initial_email_opening_probability", "email_opening_probability_multiplier_exponent", "probability_instant_email_check", "personal_email_check_process", "count" }, {"work_email_check_process", "personal_email_environments", "work_email_environments" });
		try {
			config.environments = j["environments"].get<std::vector<UserEnvironmentType>>();
			if (j.count("personal_email_environments")) {
				config.personal_email_environments = j["personal_email_environments"].get<decltype(config.personal_email_environments)>();
			}
			if (j.count("work_email_environments")) {
				config.work_email_environments = j["work_email_environments"].get<decltype(config.work_email_environments)>();
			}
			config.count = j["count"];
			config.uses_laptop = j["uses_laptop"];
			from_json(j["environment_process"], schedule, config.environment_process);
			config.probability_email_activated = j["probability_email_activated"];
			config.email_opening_probability_multiplier_exponent = j["email_opening_probability_multiplier_exponent"];
			config.initial_email_opening_probability = j["initial_email_opening_probability"];
			config.probability_instant_email_check = j["probability_instant_email_check"];
			from_json(j["personal_email_check_process"], schedule, config.personal_email_check_process);
			if (j.count("work_email_check_process")) {
				from_json(j["work_email_check_process"], schedule, config.work_email_check_process);
			}
		} catch (std::exception& e) {
			throw DeserialisationError("UserConfig", j.dump(), e);
		}
		try {
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("UserConfig", j.dump() + " (validation)", e);
		}
	}
}
