#include <cassert>
#include <boost/locale.hpp>
#include "ip_address_pool.hpp"
#include "public_connection_static_address.hpp"

namespace simbo {
	PublicConnectionStaticAddress::PublicConnectionStaticAddress(IpAddress ip_address)
		: ip_address_(ip_address) {
		if (ip_address.is_unspecified()) {
			throw std::invalid_argument(boost::locale::translate("IP address is undefined"));
		}
	}

	bool PublicConnectionStaticAddress::is_using_address_pool(const IpAddressPool& address_pool) const {
		assert(IpAddress::same_type(address_pool.min(), address_pool.max()));
		return IpAddress::same_type(ip_address_, address_pool.min()) && ip_address_ >= address_pool.min() && ip_address_ <= address_pool.max();
	}
}
