#include "exceptions.hpp"


namespace simbo {

	static std::string make_what(const std::string& input, const std::string& target_class, const std::string& original_error) {
		return (boost::locale::format("Error while deserialising\n{1}\ninto {2}:\n{3}") % input % target_class % original_error).str();
	}

	DeserialisationError::DeserialisationError(const std::string& target_class, const std::string& input, const std::exception& original_error)
		: DeserialisationError(target_class, input, original_error.what()) {
	}

	DeserialisationError::DeserialisationError(const std::string& target_class, const std::string& input, const std::string& original_error)
		: std::runtime_error(make_what(input, target_class, original_error)),
		target_class_(target_class),
		input_(input),
		original_error_(original_error) {
	}

	ConfigurationError::ConfigurationError(const std::string& configuration_section, const std::string& description)
		: std::runtime_error((boost::locale::format("Error in configuration section {1}: {2}") % configuration_section % description).str()),
		configuration_section_(configuration_section),
		description_(description) {

	}
}
