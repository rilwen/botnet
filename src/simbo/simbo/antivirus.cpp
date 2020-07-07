#include "antivirus.hpp"
#include "antivirus_config.hpp"
#include "context.hpp"
#include "host.hpp"
#include "exceptions.hpp"

namespace simbo {
	Antivirus::Antivirus(const std::string& name, std::shared_ptr<SoftwareVendor<std::string>> vendor, int release, const AntivirusConfig& config)
		: Software<std::string>(name, vendor, release),
		infection_attempt_detection_probability_(config.infection_attempt_detection_probability),
		update_at_startup_(config.update_at_startup),
		scan_at_startup_(config.scan_at_startup),
		scan_interval_(to_seconds(config.scan_interval))
	{
		if (name.empty()) {
			throw std::invalid_argument(boost::locale::translate("Empty antivirus name"));
		}
		config.validate();
	}
}