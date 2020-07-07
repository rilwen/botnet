#include <limits>
#include "exceptions.hpp"
#include "sinkhole_data_point.hpp"

namespace simbo {
	SinkholeDataPoint<1>::SinkholeDataPoint(seconds_t n_time, IpAddress n_ip, Id::id_t n_host_id, BotState n_bot_state, LocalNetworkType n_local_network_type, bool n_is_fixed)
		: time(static_cast<double>(n_time)),
		ip(n_ip.to_int64()),
		host_id(static_cast<int32_t>(n_host_id)),
		bot_state(static_cast<int8_t>(n_bot_state)),
		local_network_type(static_cast<int8_t>(n_local_network_type)),
		is_fixed(static_cast<int8_t>(n_is_fixed)),
		zero_pad_(0) {
	}

	SinkholeDataPoint<2>::SinkholeDataPoint(seconds_t n_time, IpAddress n_ip, Id::id_t n_host_id, BotState n_bot_state, LocalNetworkType n_local_network_type, bool n_is_fixed, const int n_country)
		: time(static_cast<double>(n_time)),
		ip(n_ip.to_int64()),
		host_id(static_cast<int32_t>(n_host_id)),
		bot_state(static_cast<int8_t>(n_bot_state)),
		local_network_type(static_cast<int8_t>(n_local_network_type)),
		is_fixed(static_cast<int8_t>(n_is_fixed)),
		country(static_cast<country_code_t>(n_country)) {
		if (n_country < std::numeric_limits<country_code_t>::min() || n_country > std::numeric_limits<country_code_t>::max()) {
			throw std::domain_error((boost::locale::format("Country code {1} outside the supported value range [{2}, {3}]") 
				% n_country 
				% std::numeric_limits<country_code_t>::min() 
				% std::numeric_limits<country_code_t>::max()
			).str());
		}
	}

	SinkholeDataPoint<3>::SinkholeDataPoint(seconds_t n_time, IpAddress n_ip, Id::id_t n_host_id, BotState n_bot_state, LocalNetworkType n_local_network_type, bool n_is_fixed, const int n_country, int payload_release_number)
		: time(static_cast<double>(n_time)),
		ip(n_ip.to_int64()),
		host_id(static_cast<int32_t>(n_host_id)),
		bot_state(static_cast<int8_t>(n_bot_state)),
		local_network_type(static_cast<int8_t>(n_local_network_type)),
		is_fixed(static_cast<int8_t>(n_is_fixed)),
		country(static_cast<country_code_t>(n_country)),
		payload_release(static_cast<payload_release_number_t>(payload_release_number)),
		zero_pad_{0, 0, 0, 0, 0, 0, 0}
	{
		if (n_country < std::numeric_limits<country_code_t>::min() || n_country > std::numeric_limits<country_code_t>::max()) {
			throw std::domain_error((boost::locale::format("Country code {1} outside the supported value range [{2}, {3}]")
				% n_country
				% std::numeric_limits<country_code_t>::min()
				% std::numeric_limits<country_code_t>::max()
				).str());
		}
		if (n_country < std::numeric_limits<payload_release_number_t>::min() || payload_release_number > std::numeric_limits<payload_release_number_t>::max()) {
			throw std::domain_error((boost::locale::format("Payload release number {1} outside the supported value range [{2}, {3}]")
				% payload_release_number
				% std::numeric_limits<payload_release_number_t>::min()
				% std::numeric_limits<payload_release_number_t>::max()
				).str());
		}		
	}
}