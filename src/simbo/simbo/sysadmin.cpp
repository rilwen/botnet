#include "context.hpp"
#include "date_time.hpp"
#include "exceptions.hpp"
#include "operating_system.hpp"
#include "pausing_jump_markov_process.hpp"
#include "sysadmin.hpp"

namespace simbo {
	Sysadmin::Sysadmin(std::shared_ptr<PausingJumpMarkovProcess> maintenance_trigger)
		: maintenance_trigger_(maintenance_trigger), last_maintenance_(std::make_pair(0, -infinite_time()), -infinite_time(), false) {
		if (!maintenance_trigger) {
			throw std::invalid_argument(boost::locale::translate("Null maintenance trigger process"));
		}
	}

	void Sysadmin::do_maintenance(Context& ctx) {
		if (is_time_finite(last_maintenance_.get_last_change_time())) {
			const int old_counter = last_maintenance_.get_value().first;
			last_maintenance_.evolve(ctx.get_rng(), *maintenance_trigger_, ctx.get_time());
			if (last_maintenance_.get_value().first > old_counter) {
				for (auto& host_ptr : get_hosts()) {
					if (host_ptr->is_on() && host_ptr->has_internet_access()) {
						get_logger()->debug("Performing maintenance on host {} at time {}", host_ptr->get_id(), ctx.get_time());
						host_ptr->update_operating_system_release(ctx);
					} else {
						get_logger()->debug("Skipping maintenance on host {} at time {}", host_ptr->get_id(), ctx.get_time());
					}
				}
			}
		} else {
			// Skończ setup.
			last_maintenance_.reset(std::make_pair(0, ctx.get_time()), ctx.get_time());
		}
	}
}