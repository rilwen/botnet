#include "internet_service_provider.hpp"

namespace simbo {
	InternetServiceProvider::InternetServiceProvider(std::shared_ptr<IpAddressPool> ip_address_pool)
		: ip_address_pool_(ip_address_pool) {
		if (!ip_address_pool) {
			throw std::invalid_argument(boost::locale::translate("Null IP address pool"));
		}
	}

	void InternetServiceProvider::add_connection(connection_ptr&& connection) {
		if (!connection->is_using_address_pool(*ip_address_pool_)) {
			throw std::invalid_argument(boost::locale::translate("Added connection does not use this ISPs address pool"));
		}
		add_object(*this, std::move(connection), Passkey<InternetServiceProvider>());
	}

	void InternetServiceProvider::refresh_ip_addresses(const seconds_t now) const {
		for (auto& connection : get_objects()) {
			assert(now > connection->valid_from());
			connection->refresh_ip_address(now);
		}
	}
}
