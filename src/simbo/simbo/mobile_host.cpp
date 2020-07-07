#include "context.hpp"
#include "local_network.hpp"
#include "mobile_host.hpp"

namespace simbo {
	MobileHost::MobileHost(id_t id, operating_system_ptr&& operating_system)
		: Host(id, std::move(operating_system), false), local_network_(nullptr, -infinite_time()) {
	}

	void MobileHost::set_local_network(const local_network_ptr local_network, const seconds_t time) {
		if (time <= get_local_network_change_time()) {
			throw std::domain_error(boost::locale::translate("New change time before previous one"));
		}
		if (local_network_.get_value() != local_network) {
			if (local_network_.get_value()) {
				local_network_.get_value()->remove_host(this, time, Passkey<MobileHost>());
			}
			assert(!is_time_undefined(local_network_.get_last_change_time()));
			local_network_.update(local_network, time);
			if (local_network_.get_value()) {
				local_network_.get_value()->add_host(this, time, Passkey<MobileHost>());
			}
		}
	}

	void MobileHost::update_state(Context& ctx) {		
		Host::update_state(ctx);
		if (get_activity_state().get_value() == ActivityState::ON) {
			set_local_network(update_local_network(ctx), ctx.get_time());
		}
		update_own_times(ctx.get_time());
	}
}
