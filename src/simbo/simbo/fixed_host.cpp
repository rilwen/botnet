#include <boost/locale.hpp>
#include "fixed_host.hpp"
#include "local_network.hpp"

namespace simbo {
	FixedHost::FixedHost(id_t id, operating_system_ptr&& operating_system, bool always_on, local_network_ptr local_network)
		: Host(id, std::move(operating_system), always_on),
		local_network_(local_network) {
		if (!local_network) {
			throw std::invalid_argument(boost::locale::translate("Local network is null"));
		}
		local_network->add_host(this, -infinite_time(), Passkey<FixedHost>());
	}
}
