#pragma once
#include "operating_system.hpp"

namespace simbo {
	/// Rodzina systemów operacyjnych Unix.
	class Unix : public OperatingSystem {
	public:
		bool update_at_shutdown() const override {
			return false;
		}
	protected:
		/// Konstruktor.
		/// \see OperatingSystem::OperatingSystem.
		Unix(os_spec_t spec, std::shared_ptr<vendor_t> vendor, int release)
			: OperatingSystem(spec, vendor, release) {}
	};
}