#include <boost/locale.hpp>
#include "ip_address_pool.hpp"
#include "public_connection_dynamic_address.hpp"

namespace simbo {
	PublicConnectionDynamicAddress::PublicConnectionDynamicAddress(ip_address_pool_ptr ip_address_pool, seconds_t max_address_lifetime, seconds_t creation_time)
		: ip_address_pool_(ip_address_pool),
		ip_address_(IpAddress(), -infinite_time()),
		max_address_lifetime_(max_address_lifetime) {
		if (!ip_address_pool) {
			throw std::invalid_argument(boost::locale::translate("Null IP address pool"));
		}
		if (!is_time_finite(max_address_lifetime)) {
			throw std::domain_error(boost::locale::translate("Maximum address lifetime must be finite"));
		}
		if (max_address_lifetime <= 0) {
			throw std::domain_error(boost::locale::translate("Maximum address lifetime must be positive"));
		}
		if (!is_time_finite(creation_time)) {
			throw std::domain_error(boost::locale::translate("Creation time must be finite"));
		}
		ip_address_.update(ip_address_pool_->allocate(), creation_time);		
	}

	void PublicConnectionDynamicAddress::refresh_ip_address(const seconds_t time_now) {		
		if (time_now > valid_from()) {
			assert(!ip_address_.get_value().is_unspecified());
			if (time_now >= ip_address_.get_time() + max_address_lifetime_) {
				ip_address_.update(ip_address_pool_->update(ip_address_.get_value()), time_now);
			}
		} else {
			throw std::domain_error(boost::locale::translate("refresh_ip_address() called with time_now not after valid_from() value"));
		}		
	}

	bool PublicConnectionDynamicAddress::is_using_address_pool(const IpAddressPool& address_pool) const {
		return ip_address_pool_.get() == &address_pool;
	}
}
