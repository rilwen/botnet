#include <boost/locale.hpp>
#include "unique_ip_address_pool.hpp"

namespace simbo {
	template <class Addr> UniqueIpAddressPool<Addr>::UniqueIpAddressPool(const addr_t& min, const addr_t& max)
		: min_(min),
		max_(max),
		min_as_bytes_(min.to_bytes()),
		size_(IpAddress::range_size(min, max)),
		is_available_(size_),
		allocated_size_(0) {
		// Ustaw wszystkie bity na 1.
		is_available_.flip();
		assert(is_available_.all());
	}

	template <class Addr> IpAddress UniqueIpAddressPool<Addr>::allocate() {
		if (allocated_size() < total_size()) {
			const auto i = is_available_.find_first();
			is_available_.flip(i);
			++allocated_size_;
			bytes_t new_addr(min_as_bytes_);
			advance(new_addr, i);
			return IpAddress(addr_t(new_addr));
		} else {
			throw std::logic_error(boost::locale::translate("No addresses available"));
		}
	}

	template <class Addr> void UniqueIpAddressPool<Addr>::release(const IpAddress& released) {
		if (!IpAddress::same_type(min_, released)) {
			throw std::invalid_argument(boost::locale::translate("Released address has wrong type"));
		}
		if (released < min_ || released > max_) {
			throw std::invalid_argument(boost::locale::translate("Released address outside range"));
		}
		const int64_t dist = IpAddress::distance(min_, released);
		if (is_available_[dist]) {
			throw std::invalid_argument(boost::locale::translate("Released address was not reserved"));
		}
		is_available_.flip(dist);
		--allocated_size_;
	}

	// Konkretyzacja szablonów.
	template class UniqueIpAddressPool<boost::asio::ip::address_v4>;
	template class UniqueIpAddressPool<boost::asio::ip::address_v6>;

	std::unique_ptr<IpAddressPool> make_unique_ip_address_pool(const IpAddress& min, const IpAddress& max) {
		if (!IpAddress::same_type(min, max)) {
			throw std::invalid_argument(boost::locale::translate("Minimum and maximum address have different type"));
		}
		if (min.is_v4()) {
			return std::make_unique<UniqueIpAddressPoolV4>(min.to_v4(), max.to_v4());
		} else {
			assert(min.is_v6());
			return std::make_unique<UniqueIpAddressPoolV6>(min.to_v6(), max.to_v6());
		}
	}
}
