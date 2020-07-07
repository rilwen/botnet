#include "simbo/piecewise_constant_interpolator1d.hpp"
#include "mock_operating_system.hpp"

namespace simbo {
	static const OperatingSystemType default_type = OperatingSystemType::OTHER;
	static const int default_version = 1;
	static const int default_release = 1;

	static std::shared_ptr<OperatingSystem::vendor_t> make_mock_vendor(OperatingSystemType os_typ, int version, int release) {
		OperatingSystem::vendor_t::config_t cfg;
		cfg.release_schedules[std::make_pair(os_typ, version)] = OperatingSystem::vendor_t::config_t::release_schedule_t(PiecewiseConstantInterpolator1D<DateTime, int>(min_datetime(), release));
		return std::make_shared<OperatingSystem::vendor_t>("Miko-Soft", cfg);
	}

	MockOS::MockOS()
		: MockOS(std::make_pair(default_type, default_version)) {}

	MockOS::MockOS(os_spec_t spec)
		: OperatingSystem(spec, make_mock_vendor(default_type, default_version, default_release), default_release) {}
}
