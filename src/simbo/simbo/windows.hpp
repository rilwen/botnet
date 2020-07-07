#pragma once
#include "operating_system.hpp"

namespace simbo {
	/// Rodzina systemów operacyjnych Windows.
	class Windows : public OperatingSystem {
	public:
		/*!
		\param vendor Dostawca systemu.
		\param version Numer wersji systemu.
		\param release Numer edycji systemu.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		Windows(std::shared_ptr<vendor_t> vendor, int version, int release)
			: OperatingSystem(std::make_pair(OperatingSystemType::WINDOWS, version), vendor, release) {}

		bool update_at_shutdown() const override {			
			return true;
		}
	};
}
