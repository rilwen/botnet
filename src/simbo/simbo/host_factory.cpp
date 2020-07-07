#include <stdexcept>
#include <boost/locale.hpp>
#include "desktop.hpp"
#include "host_factory.hpp"
#include "laptop.hpp"
#include "server.hpp"
#include "sysadmin.hpp"

namespace simbo {
	Desktop* HostFactory::make_desktop(Sysadmin& sysadmin, operating_system_ptr&& os, bool always_on, LocalNetwork* local_network) {
		auto host = std::make_unique<Desktop>(id_factory_.get_next_good_id(), std::move(os), always_on, local_network, Passkey<HostFactory>());
		auto host_ptr = host.get();
		sysadmin.add_host(std::move(host));
		return host_ptr;
	}

	Server* HostFactory::make_server(Sysadmin& sysadmin, operating_system_ptr&& os, LocalNetwork* local_network) {
		auto host = std::make_unique<Server>(id_factory_.get_next_good_id(), std::move(os), local_network, Passkey<HostFactory>());
		auto host_ptr = host.get();
		sysadmin.add_host(std::move(host));
		return host_ptr;
	}

	Laptop* HostFactory::make_laptop(Sysadmin& sysadmin, operating_system_ptr&& os, Laptop::local_network_switcher_t&& local_network_switcher) {
		auto host = std::make_unique<Laptop>(id_factory_.get_next_good_id(), std::move(os), std::move(local_network_switcher), Passkey<HostFactory>());
		auto host_ptr = host.get();
		sysadmin.add_host(std::move(host));
		return host_ptr;
	}
}
