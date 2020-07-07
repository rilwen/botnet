#include "n6_data_point.hpp"

namespace simbo {
	n6DataPoint<1>::n6DataPoint(seconds_t n_time, unsigned int n_count, seconds_t n_until, IpAddress n_ip, const int n_country)
		: ip(n_ip.to_int64()),
		time(static_cast<time_t>(n_time)),
		until(static_cast<time_t>(n_until)),
		count(static_cast<uint16_t>(n_count)),
		country(static_cast<country_code_t>(n_country)) {
		if (n_country < std::numeric_limits<country_code_t>::min() || n_country > std::numeric_limits<country_code_t>::max()) {
			throw std::domain_error((boost::locale::format("Country code {1} outside the supported value range [{2}, {3}]")
				% n_country
				% std::numeric_limits<country_code_t>::min()
				% std::numeric_limits<country_code_t>::max()
				).str());
		}
	}
}