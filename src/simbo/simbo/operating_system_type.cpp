#include <cassert>
#include "exceptions.hpp"
#include "json.hpp"
#include "operating_system_type.hpp"

namespace simbo {
	void to_string(std::string& str, const OperatingSystemType ost) {
		if (ost == OperatingSystemType::ANDROID) {
			str = "ANDROID";
		} else if (ost == OperatingSystemType::LINUX) {
			str = "LINUX";
		} else if (ost == OperatingSystemType::MACOS) {
			str = "MACOS";
		} else if (ost == OperatingSystemType::WINDOWS) {
			str = "WINDOWS";
		} else {
			assert(ost == OperatingSystemType::OTHER);
			str = "OTHER";
		}
	}

	void from_string(const std::string& str, OperatingSystemType& ost) {
		if (str == "ANDROID") {
			ost = OperatingSystemType::ANDROID;
		} else if (str == "LINUX") {
			ost = OperatingSystemType::LINUX;
		} else if (str == "MACOS") {
			ost = OperatingSystemType::MACOS;
		} else if (str == "WINDOWS") {
			ost = OperatingSystemType::WINDOWS;
		} else if (str == "OTHER") {
			ost = OperatingSystemType::OTHER;
		} else {
			throw std::invalid_argument((boost::locale::format("Unknown operating system type: {1}") % str).str());
		}
	}

	void to_json(json& j, OperatingSystemType ost) {
		std::string str;
		to_string(str, ost);
		j = str;
	}

	void from_json(const json& j, OperatingSystemType& ost) {
		std::string str = j;
		try {
			from_string(str, ost);
		} catch (std::exception& e) {
			throw DeserialisationError("OperatingSystemType", j.dump(), e);
		}
	}
}
