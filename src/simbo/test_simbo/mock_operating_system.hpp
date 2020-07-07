#pragma once
#include "simbo/operating_system.hpp"

namespace simbo {
	/// Udawany OperatingSystem.
	class MockOS : public OperatingSystem {
	public:
		MockOS();

		MockOS(os_spec_t spec);

		bool update_at_shutdown() const override {
			return false;
		}		
	};
}
