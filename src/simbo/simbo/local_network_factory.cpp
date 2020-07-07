#include "local_network_factory.hpp"
#include "network_admin.hpp"

namespace simbo {
	LocalNetwork* LocalNetworkFactory::make_local_network(NetworkAdmin& network_admin, LocalNetworkType typ, int country) {
		auto ln = std::make_unique<LocalNetwork>(id_factory_.get_next_good_id(), typ, country);
		auto ln_ptr = ln.get();
		network_admin.add_network(std::move(ln));
		return ln_ptr;
	}
}