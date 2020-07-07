#include "ip_address.hpp"
#include "json.hpp"

namespace simbo {
	IpAddress::IpAddress():
		boost::asio::ip::address() {
	}

	int64_t IpAddress::to_int64() const {
		if (is_v4()) {
			return to_int64(to_v4().to_bytes());
		} else {
			assert(is_v6());
			return to_int64(to_v6().to_bytes());
		}
	}

	int64_t IpAddress::distance(const IpAddress& adr1, const IpAddress& adr2) {
		if (!same_type(adr1, adr2)) {
			throw std::invalid_argument(boost::locale::translate("Addresses are of different types"));
		}
		if (adr1 > adr2) {
			return distance(adr2, adr1);
		}
		int64_t dist;
		if (adr1.is_v4()) {
			const auto bytes1 = adr1.to_v4().to_bytes();
			const auto bytes2 = adr2.to_v4().to_bytes();
			const auto diff = sub(bytes2, bytes1);
			dist = to_int64(diff);
		} else {
			assert(adr1.is_v6());
			const auto bytes1 = adr1.to_v6().to_bytes();
			const auto bytes2 = adr2.to_v6().to_bytes();
			const auto diff = sub(bytes2, bytes1);
			dist = to_int64(diff);
		}
		return dist;
	}

	int64_t IpAddress::range_size(const IpAddress& min, const IpAddress& max) {
		if (min > max) {
			throw std::invalid_argument(boost::locale::translate("Minimum address above maximum"));
		}
		const uint64_t size = static_cast<uint64_t>(distance(min, max)) + 1;
		if (size > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
			throw std::range_error(boost::locale::translate("Range size overflows int64_t type"));
		}
		return static_cast<int64_t>(size);
	}

	bool IpAddress::same_type(const IpAddress& adr1, const IpAddress& adr2) {
		return adr1.is_v4() && adr2.is_v4() || adr1.is_v6() && adr2.is_v6();
	}

	template <size_t N> std::array<unsigned char, N> IpAddress::sub(const std::array<unsigned char, N>& a, const std::array<unsigned char, N>& b) {
		std::array<unsigned char, N> c;
		int r = 0;
		// Odejmowanie w słupku.
		for (int i = static_cast<int>(N - 1); i >= 0; --i) {
			assert(r <= 0);
			assert(r >= -1);
			int s = a[i] - b[i] + r;
			if (s < 0) {
				assert(s >= -256);
				s += 256;
				r = -1;
			} else {
				r = 0;
			}
			c[i] = s;
		}
		if (r) {
			throw std::invalid_argument(boost::locale::translate("a < b"));
		}
		return c;
	}

	void from_json(const json& j, IpAddress& ip_address) {
		try {
			std::string ip_address_str = j;
			ip_address = IpAddress::from_string(ip_address_str);
		} catch (const std::exception& e) {
			throw DeserialisationError("IpAddress", j.dump(), e);
		}
	}
}
