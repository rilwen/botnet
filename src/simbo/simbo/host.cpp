#include <cassert>
#include <limits>
#include <boost/locale.hpp>
#include "action_interval.hpp"
#include "antivirus.hpp"
#include "botnet.hpp"
#include "context.hpp"
#include "host.hpp"
#include "local_network.hpp"
#include "log.hpp"
#include "rng.hpp"
#include "operating_system.hpp"
#include "sysadmin.hpp"

namespace simbo {
	Host::Host(id_t id, operating_system_ptr&& operating_system, bool always_on)
		: id_(id),		
		always_on_(always_on),
		activity_state_(always_on ? ActivityState::ON : ActivityState::OFF, -infinite_time()),
		infection_state_(InfectionState::NOT_INFECTED, -infinite_time()),
		bot_state_(BotState::NONE, -infinite_time()),
		last_ping_time_(-infinite_time()),
		uptime_since_last_ping_(-infinite_time()),
		online_time_since_joining_botnet_(-infinite_time()),
		user_counter_(0),
		av_last_scan_time_(-infinite_time()) {
		if (!Id::is_good(id)) {
			throw std::domain_error(boost::locale::translate("Good ID required"));
		}
		if (!operating_system) {
			throw std::invalid_argument(boost::locale::translate("Operating system cannot be null"));
		}
		os_ = std::move(operating_system);
	}

	void Host::turn_on(Context& ctx, seconds_t time) {
		if (!is_always_on()) {
			set_activity_state(ctx, time, ActivityState::ON);
		}
	}

	void Host::turn_off(Context& ctx, seconds_t time) {
		if (!is_always_on()) {
			set_activity_state(ctx, time, ActivityState::OFF);
		}
	}

	void Host::set_activity_state(Context& ctx, const seconds_t time, const ActivityState new_state) {
		if (!ctx.is_inside_current_time_step(time)) {
			throw std::domain_error((boost::locale::format("Activity change time {1} outside allowed range [{2}, {3})") % time % ctx.get_time() % ctx.get_next_time()).str());
		}
		update_own_times(time);
		if (is_on() && new_state == ActivityState::OFF && os_->update_at_shutdown() && has_internet_access()) {
			get_logger()->debug("Updating OS release at shutdown on host {}", id_);
			update_operating_system_release(ctx);
		}
		const auto old_state = activity_state_.get_value();
		activity_state_.update(new_state, time);
		get_tracker()->info("{},Host,{},ActivityState,{}", time, get_id(), to_string(new_state));
		if (is_on() && old_state == ActivityState::OFF) {
			if (av_) {
				bool av_was_updated = false;
				if (av_->get_update_at_startup() && has_internet_access()) {
					av_was_updated = av_->update_release_number(ctx.get_datetime());
				}
				if (av_was_updated) {
					get_logger()->debug("Updated AV release on host {} to {} at {}", id_, av_->get_release_number(), time);
				}
				if ((av_was_updated && av_->get_scan_after_update()) || av_->get_scan_at_startup()) {
					perform_antivirus_scan(ctx);
				}
			}
			if (get_infection_state().get_value() == InfectionState::INFECTED && has_internet_access() && has_botnet_role()) {
				update_bot_state(ctx, time);
			}
		}
		update_own_times(time);
	}

	bool Host::has_internet_access() const {
		return get_local_network() != nullptr && get_local_network()->get_public_connection() != nullptr;
	}

	void Host::update_state(Context& ctx) {
		update_own_times(ctx.get_time());
		if (is_on()) {
			if (av_ && (ctx.get_time() - av_last_scan_time_) >= av_->get_scan_interval()) {
				if (has_internet_access()) {
					if (av_->update_release_number(ctx.get_datetime())) {						
						get_logger()->debug("Updated AV release on host {} to {} at {}", id_, av_->get_release_number(), ctx.get_time());
					}					
				}
				perform_antivirus_scan(ctx);
			}
			if (get_infection_state().get_value() == InfectionState::INFECTED
				&& has_internet_access()) {
				if (has_botnet_role()) {
					update_bot_state(ctx, ctx.get_time());
				} else {
					// Host został wcześniej zainfekowany, ale dopiero teraz ma dostęp do internetu.
					join_botnet(ctx, ctx.get_time());
				}
			}
		}
		update_own_times(ctx.get_time());
	}

	void Host::update_own_times(const seconds_t wall_time) {
		const bool is_up = is_on();
		bot_state_.update_own_time(wall_time, is_up);
		uptime_since_last_ping_.update(wall_time, is_up);		
		online_time_since_joining_botnet_.update(wall_time, is_up && has_internet_access());
		get_logger()->debug("Host {} updated own times to {}", id_, wall_time);
	}

	void Host::update_operating_system_release(Context& ctx) {
		assert(is_on());
		assert(has_internet_access());
		if (os_->update_release_number(ctx.get_datetime())) {
			get_logger()->debug("Updated OS release on host {} to {} at {}", id_, os_->get_release_number(), ctx.get_time());
		}
	}

	void Host::perform_antivirus_scan(Context& ctx) {
		assert(is_on());
		assert(av_);		
		if (get_infection_state().get_value() == InfectionState::INFECTED && has_botnet_role()) {
			const auto& payload_config = ctx.get_botnet_config().payload.interpolate_and_extrapolate(payload_release_number_);
			const auto it = payload_config.min_detecting_av_release.find(av_->get_spec());
			if (it != payload_config.min_detecting_av_release.end()) {
				if (it->second <= av_->get_release_number()) {
					cure(ctx);
					get_logger()->debug("AV cured host {} from infection as of {}", id_, to_string(ctx.get_datetime()));
				}
			}
		}		
		av_last_scan_time_ = ctx.get_time();
		get_logger()->debug("Performed AV scan on host {} at {}", id_, ctx.get_time());
	}

	/** Funkcje związane z botnetem. **/

	void Host::try_infecting_via_email(Context& ctx, const seconds_t time) {
		assert(is_on());
		bool attempt_successful = get_infection_state().get_value() == InfectionState::NOT_INFECTED && ctx.get_botnet().is_infectable(get_operating_system(), av_.get());
		if (attempt_successful && av_) {
			const double p_detect = av_->get_infection_attempt_detection_probability();
			if (p_detect > 0) {
				if (p_detect < 1) {
					attempt_successful = ctx.get_rng().draw_uniform() >= p_detect;
				} else {
					attempt_successful = false;
				}
			}
		}
		if (attempt_successful) {
			infect(ctx, time);
		}
	}

	void Host::infect(Context& ctx, const seconds_t time) {
		assert(is_on());
		if (!ctx.is_inside_current_time_step(time)) {
			throw std::domain_error(boost::locale::translate("Infection time outside allowed range"));
		}
		transition_infection_state(InfectionState::INFECTED, InfectionState::NOT_INFECTED, time);
		if (has_internet_access()) {
			join_botnet(ctx, time);
		}
		get_logger()->debug("Infected host {} at {}", id_, time);
	}

	void Host::cure(Context& ctx) {
		assert(is_on());
		transition_infection_state(InfectionState::NOT_INFECTED, InfectionState::INFECTED, ctx.get_time());
		if (has_botnet_role()) {
			leave_botnet(ctx);
		}
		get_logger()->debug("Cured host {} at {}", id_, ctx.get_time());
	}

	void Host::reset(Context& ctx) {
		assert(is_on());
		const bool was_infected = infection_state_.get_value() == InfectionState::INFECTED;
		infection_state_.update(InfectionState::NOT_INFECTED, ctx.get_time());
		get_tracker()->info("{},Host,{},InfectionState,{}", ctx.get_time(), get_id(), to_string(InfectionState::NOT_INFECTED));
		if (was_infected && has_botnet_role()) {
			leave_botnet(ctx);
		}
		get_logger()->debug("Reset host {} at {}", id_, ctx.get_time());
	}

	void Host::transition_infection_state(const InfectionState new_state, const InfectionState required_present_state, seconds_t time) {
		assert(is_on());
		if (infection_state_.get_value() == required_present_state) {
			infection_state_.update(new_state, time);
			get_tracker()->info("{},Host,{},InfectionState,{}", time, get_id(), to_string(new_state));
		} else {
			throw std::logic_error((boost::locale::format("Required host state {1} but {2} instead") % required_present_state % infection_state_.get_value()).str());
		}
	}

	void Host::update_bot_state(Context& ctx, seconds_t time) {
		if (!ctx.is_inside_current_time_step(time)) {
			throw std::domain_error(boost::locale::translate("Bot state update time outside allowed range"));
		}
		assert(is_on());
		assert(has_internet_access());
		assert(infection_state_.get_value() == InfectionState::INFECTED);
		assert(has_botnet_role());
		const BotnetConfig& botnet_config = ctx.get_botnet_config();
		if (bot_needs_reconfiguration(botnet_config, time)) {
			reconfigure_bot(ctx, time);
		} else {
			// Sprawdź, czy host nie musi już pingować botnet.
			const std::vector<seconds_t> new_ping_times(get_new_pings_times(botnet_config, time));			
			if (!new_ping_times.empty()) {
				get_logger()->debug("Host {} sending {} pings", id_, new_ping_times.size());
				for (seconds_t ping_time : new_ping_times) {
					ping_botnet(ctx, ping_time);
				}				
			}
		}
	}

	void Host::reconfigure_bot(Context& ctx, seconds_t time) {
		// Poproś botnet o nową konfigurację. 
		set_bot_state(ctx.get_botnet().configure_host(ctx, this, time), time);
		// Rekonfiguracja również liczy się jako ping.
		update_ping_time(time);
		get_logger()->debug("Host {} asked for new configuration at {}", id_, time);
	}

	void Host::ping_botnet(Context& ctx, seconds_t time) {
		// Zawiadom botnet, że żyjesz.
		update_ping_time(time);
		ctx.get_botnet().receive_ping(this, time);
		get_logger()->debug("Host {} pinged botnet at {}", id_, time);
	}

	void Host::set_bot_state(BotState new_state, seconds_t time) {
		bot_state_.update(new_state, time, is_on());
		get_tracker()->info("{},Host,{},BotState,{}", time, get_id(), to_string(new_state));
	}

	bool Host::bot_needs_reconfiguration(const BotnetConfig& botnet_config, const seconds_t time) const {
		assert(is_on());
		assert(has_internet_access());
		assert(infection_state_.get_value() == InfectionState::INFECTED);
		assert(has_botnet_role());
		if (bot_state_.get_value() == BotState::SETTING_UP) {
			// Sprawdź czy host skończył ściągać payload i jest gotów przyjąć nową rolę (worker, propagator, itd.).
			return online_time_since_joining_botnet_.get_own_time() >= botnet_config.dropper.setting_up_period;
		} else {
			// Sprawdź czy host nie musi już się rekonfigurować w botnecie.
			const auto& payload_config = botnet_config.payload.interpolate_and_extrapolate(payload_release_number_);
			if (activity_state_.get_last_change_time() == time && payload_config.reconfigure_at_start) {
				return true;
			}
			if (payload_config.reconfiguration_interval.is_by_wall_clock()) {
				return bot_state_.get_last_update_time() + payload_config.reconfiguration_interval.get_period() <= time;
			} else {
				return bot_state_.get_own_time_since_last_update() >= payload_config.reconfiguration_interval.get_period();
			}
		}
	}

	std::vector<seconds_t> Host::get_new_pings_times(const BotnetConfig& botnet_config, const seconds_t time) const {
		assert(is_on());
		assert(has_internet_access());
		assert(infection_state_.get_value() == InfectionState::INFECTED);
		assert(has_botnet_role());
		const auto& payload_config = botnet_config.payload.interpolate_and_extrapolate(payload_release_number_);		
		if (activity_state_.get_last_change_time() == time && payload_config.ping_at_start) {
			get_logger()->debug("Host {} pinging botnet after starting at {}", id_, time);
			return std::vector<seconds_t>({ time });
		}
		seconds_t time_not_pinged;
		if (payload_config.ping_interval.is_by_wall_clock()) {
			time_not_pinged = time - last_ping_time_;
		} else {
			time_not_pinged = uptime_since_last_ping_.get_own_time();
		}
		const int number_pings = static_cast<int>(time_not_pinged / payload_config.ping_interval.get_period());
		std::vector<seconds_t> ping_times(number_pings);
		for (int i = 0; i < number_pings; ++i) {
			ping_times[i] = time - (number_pings - 1 - i) * payload_config.ping_interval.get_period();
		}
		return ping_times;
	}

	void Host::update_ping_time(seconds_t wall_time) {
		last_ping_time_ = wall_time;
		uptime_since_last_ping_.reset(wall_time);
		get_logger()->debug("Host {} updated ping time to {}", id_, wall_time);
	}

	void Host::join_botnet(Context& ctx, seconds_t time) {
		assert(is_on());
		assert(has_internet_access());
		assert(infection_state_.get_value() == InfectionState::INFECTED);
		set_bot_state(BotState::SETTING_UP, time);
		payload_release_number_ = ctx.get_botnet_config().payload_release_schedule.interpolate_and_extrapolate(ctx.get_datetime());
		online_time_since_joining_botnet_.reset(time);
		update_ping_time(time);
		ctx.get_botnet().add_host(ctx, this, time);
		get_logger()->debug("Host {} joined botnet at {}", id_, time);
	}

	void Host::leave_botnet(Context& ctx) {
		assert(is_on());
		assert(infection_state_.get_value() == InfectionState::NOT_INFECTED);
		set_bot_state(BotState::NONE, ctx.get_time());
		ctx.get_botnet().remove_host(this, ctx.get_time());
		get_logger()->debug("Host {} left botnet at {}", id_, ctx.get_time());
	}

	int Host::get_payload_release_number() const {
		if (!has_botnet_role()) {
			throw std::logic_error(boost::locale::translate("Host is not in botnet"));
		}
		return payload_release_number_;
	}
}
