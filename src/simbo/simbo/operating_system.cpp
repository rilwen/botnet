#include "android.hpp"
#include "exceptions.hpp"
#include "linux.hpp"
#include "macos.hpp"
#include "operating_system.hpp"
#include "windows.hpp"

namespace simbo {
	OperatingSystem::OperatingSystem(os_spec_t spec, std::shared_ptr<vendor_t> vendor, int release)
		: Software<os_spec_t>(spec, vendor, release) {}

	std::unique_ptr<OperatingSystem> OperatingSystem::build(std::shared_ptr<vendor_t> vendor, const os_spec_t os_spec, const int release) {
		const auto typ = os_spec.first;
		const int version = os_spec.second;
		if (typ == OperatingSystemType::ANDROID) {
			return std::make_unique<Android>(vendor, version, release);
		} else if (typ == OperatingSystemType::LINUX) {
			return std::make_unique<Linux>(vendor, version, release);
		} else if (typ == OperatingSystemType::MACOS) {
			return std::make_unique<MacOs>(vendor, version, release);
		} else if (typ == OperatingSystemType::WINDOWS) {
			return std::make_unique<Windows>(vendor, version, release);
		} else {
			throw std::invalid_argument((boost::locale::format("Cannot create operating system of type {1}") % typ).str());
		}
	}
}
