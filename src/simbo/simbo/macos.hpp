#pragma once
#include "unix.hpp"

namespace simbo {
	/// System operacyjny macOS.
	class MacOs : public Unix {
	public:
		/*!
		\param vendor Dostawca systemu.
		\param version Numer wersji systemu.
		\param release Numer edycji systemu.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		MacOs(std::shared_ptr<vendor_t> vendor, int version, int release)
			: Unix(std::make_pair(OperatingSystemType::MACOS, version), vendor, release) {}
	};
}
