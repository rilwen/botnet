#include "context.hpp"
#include "exceptions.hpp"
#include "laptop.hpp"
#include "markov_process.hpp"

namespace simbo {
	Laptop::Laptop(id_t id, operating_system_ptr&& operating_system, local_network_switcher_t&& local_network_switcher, Passkey<HostFactory>)
		: MobileHost(id, std::move(operating_system)),
		local_network_switcher_(std::move(local_network_switcher)) {
	}

	Laptop::local_network_ptr Laptop::update_local_network(Context& ctx) {
		return local_network_switcher_.update(ctx.get_rng(), ctx.get_time());
	}
}
