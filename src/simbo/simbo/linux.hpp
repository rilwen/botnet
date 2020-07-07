#pragma once
#include "unix.hpp"

namespace simbo {
	/// System operacyjny Linux.
	class Linux: public Unix {
	public:
		/*!
		\param vendor Dostawca systemu.
		\param version Numer wersji systemu.
		\param release Numer edycji systemu.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		Linux(std::shared_ptr<vendor_t> vendor, int version, int release)
			: Unix(std::make_pair(OperatingSystemType::LINUX, version), vendor, release) {}
	};
}
