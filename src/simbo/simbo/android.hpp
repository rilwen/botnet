#pragma once
#include "operating_system.hpp"

namespace simbo {
	/// System operacyjny Android.
	class Android : public OperatingSystem {
	public:
		/*!
		\param vendor Dostawca systemu.
		\param version Numer wersji systemu.
		\param release Numer edycji systemu.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		Android(std::shared_ptr<vendor_t> vendor, int version, int release)
			: OperatingSystem(std::make_pair(OperatingSystemType::ANDROID, version), vendor, release) {}

		bool update_at_shutdown() const override {
			return false;
		}
	};
}
