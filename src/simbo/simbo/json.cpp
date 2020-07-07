#include "json.hpp"

namespace simbo {
	void expect_key(const json& j, const std::string& target_class, const std::string& key) {
		if (!j.count(key)) {
			throw DeserialisationError(target_class, j.dump(), (boost::locale::format("Required field \"{1}\" missing") % key).str());
		}
	}

	void expect_object(const json& j, const std::string& target_class) {
		if (!j.is_object()) {
			throw simbo::DeserialisationError(target_class, j.dump(), boost::locale::translate("Expected a JSON object"));
		}
	}

	void validate_keys(const json& j, const std::string& target_class, std::initializer_list<std::string> expected_keys,
		std::initializer_list<std::string> optional_keys) {
		expect_object(j, target_class);
		for (const std::string& expected_key : expected_keys) {
			expect_key(j, target_class, expected_key);
		}
		for (auto it = j.begin(); it != j.end(); ++it) {
			const auto& key = it.key();
			if (std::find(expected_keys.begin(), expected_keys.end(), key) == expected_keys.end()
				&& std::find(optional_keys.begin(), optional_keys.end(), key) == optional_keys.end()) {
				throw DeserialisationError(target_class, j.dump(), (boost::locale::format("Field \"{1}\" not expected") % key).str());
			}
		}
	}
}
