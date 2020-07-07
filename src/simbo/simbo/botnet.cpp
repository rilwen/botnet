#include "antivirus.hpp"
#include "botnet.hpp"
#include "context.hpp"
#include "exceptions.hpp"
#include "host.hpp"
#include "operating_system.hpp"
#include "rng.hpp"
#include "sparse_discrete_random_variable.hpp"

namespace simbo {

	Botnet::Botnet(const BotnetConfig& config, Sinkhole&& sinkhole)
		: config_(config), sinkhole_(std::move(sinkhole)) {
		config.validate();
	}

	void Botnet::add_host(const Context& ctx, Host* host, seconds_t time) {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Host is null"));
		}
		if (!host->is_on()) {
			throw std::invalid_argument(boost::locale::translate("Host is off"));
		}
		if (!host->has_internet_access()) {
			throw std::invalid_argument(boost::locale::translate("Host is not online"));
		}
		if (host->get_infection_state().get_value() != InfectionState::INFECTED) {
			throw std::invalid_argument(boost::locale::translate("Host is not infected"));
		}
		if (host->get_bot_state().get_value() != BotState::SETTING_UP) {
			throw std::invalid_argument(boost::locale::translate("Bot state should be BotState::SETTING_UP"));
		}
		if (contains_host(host)) {
			throw std::invalid_argument(boost::locale::translate("Host already in botnet"));
		}
		// Dodanie hosta liczy się jako wiadomość.
		register_bot_message(host, time);
		hosts_.insert(host);
		// Podbij licznik dla danej edycji.
		const int payload_release_number = config_.payload_release_schedule.interpolate_and_extrapolate(ctx.get_datetime());
		const auto it = host_counts_per_payload_release_.find(payload_release_number);
		if (it == host_counts_per_payload_release_.end()) {
			host_counts_per_payload_release_.insert(std::make_pair(payload_release_number, 1));
		} else {
			++it->second;
		}
	}

	void Botnet::remove_host(Host* host, seconds_t time) {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Host is null"));
		}
		if (!host->is_on()) {
			throw std::invalid_argument(boost::locale::translate("Host is off"));
		}
		if (host->get_infection_state().get_value() == InfectionState::INFECTED) {
			throw std::invalid_argument(boost::locale::translate("Host is infected"));
		}
		if (host->get_bot_state().get_value() != BotState::NONE) {
			throw std::invalid_argument(boost::locale::translate("Bot state should be BotState::NONE"));
		}
		if (!contains_host(host)) {
			throw std::invalid_argument(boost::locale::translate("Host not in botnet"));
		}
		hosts_.erase(host);
	}

	simbo::BotState Botnet::configure_host(Context& ctx, Host* host, seconds_t time) {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Host is null"));
		}
		if (!host->is_on()) {
			throw std::invalid_argument(boost::locale::translate("Host is off"));
		}
		if (host->get_infection_state().get_value() != InfectionState::INFECTED) {
			throw std::invalid_argument(boost::locale::translate("Host is not infected"));
		}
		if (!host->has_internet_access()) {
			throw std::invalid_argument(boost::locale::translate("Host is not online"));
		}
		if (!host->has_botnet_role()) {
			throw std::invalid_argument(boost::locale::translate("Host has no botnet role"));
		}
		if (!contains_host(host)) {
			throw std::invalid_argument(boost::locale::translate("Host not in botnet"));
		}
		register_bot_message(host, time);
		bool draw_new_role = true;
		if (host->get_bot_state().get_value() != BotState::SETTING_UP) {
			const double probability_of_reconfiguration = config_.probability_of_reconfiguration.interpolate_and_extrapolate(ctx.get_datetime());
			if (probability_of_reconfiguration < 1) {
				draw_new_role &= ctx.get_rng().draw_uniform() < probability_of_reconfiguration;
			}
		}
		if (draw_new_role) {
			const auto new_role_rv = config_.bot_state_weights.interpolate_and_extrapolate(ctx.get_datetime());
			return (*new_role_rv)(ctx.get_rng());
		} else {
			return host->get_bot_state().get_value();
		}
	}

	void Botnet::receive_ping(const Host* const host, const seconds_t time) {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Host is null"));
		}
		if (!host->is_on()) {
			throw std::invalid_argument(boost::locale::translate("Host is off"));
		}
		if (host->get_infection_state().get_value() != InfectionState::INFECTED) {
			throw std::invalid_argument(boost::locale::translate("Host is not infected"));
		}
		if (!host->has_internet_access()) {
			throw std::invalid_argument(boost::locale::translate("Host is not online"));
		}
		if (!host->has_botnet_role()) {
			throw std::invalid_argument(boost::locale::translate("Host has no botnet role"));
		}
		if (!contains_host(host)) {
			throw std::invalid_argument(boost::locale::translate("Host not in botnet"));
		}
		register_bot_message(host, time);
	}

	int Botnet::calc_number_actively_propagating_bots(const int release_number) const
{
		int number = 0;
		for (const Host* host : hosts_) {
			assert(host);
			if (host->is_on() && host->has_internet_access() && host->get_bot_state().get_value() == BotState::PROPAGATING
				&& host->get_payload_release_number() == release_number) {
				++number;
			}
		}
		return number;
	}

	bool Botnet::contains_host(const Host* host) const {
		// const_cast po to, żeby sprawdzić czy host należy do botnetu.
		return hosts_.count(const_cast<Host*>(host));
	}

	void Botnet::register_bot_message(const Host* host, seconds_t time) {
		sinkhole_.add_data_point(sinkhole_.get_data_from_bot(host, time));
	}

	bool Botnet::is_infectable(const OperatingSystem& os, const Antivirus* av) const {
		// Jeżeli botnet nie "wie" o danym systemie operacyjnym, to nie może go zainfekować.
		bool os_is_infectable = false;		
		const auto max_release_it = config_.dropper.max_infected_release.find(os.get_spec());
		if (max_release_it != config_.dropper.max_infected_release.end()) {
			os_is_infectable = os.get_release_number() <= max_release_it->second;
		}

		/// Antywirus może wykryć próbę infekcji za pomocą heurystyk albo później, podczas skanowania.

		//// Jeżeli antywirus nie "wie" o botnecie, to nie może go wykryć.
		//bool av_can_detect = false;
		//if (av) {
		//	const auto av_min_detect_release_it = payload_config.min_detecting_av_release.find(av->get_spec());
		//	if (av_min_detect_release_it != payload_config.min_detecting_av_release.end()) {
		//		av_can_detect = av->get_release_number() >= av_min_detect_release_it->second;
		//	}
		//}
		return os_is_infectable;
	}

	std::vector<double> Botnet::bot_state_weights_as_vector(const std::unordered_map<BotState, double>& map) {
		if (map.empty()) {
			throw std::invalid_argument(boost::locale::translate("Weight map is empty"));
		}
		const int start = static_cast<int>(BotState::SETTING_UP) + 1;
		const int end = static_cast<int>(BotState::NONE);
		std::vector<double> weights;
		weights.reserve(end - start);
		for (int i = start; i < end; ++i) {
			const BotState s = static_cast<BotState>(i);
			const auto it = map.find(s);
			double w = 0.;
			if (it != map.end()) {
				w = it->second;
			}
			weights.push_back(w);
		}
		return weights;
	}

	bool Botnet::can_detect_and_remove(const Antivirus& av, const int payload_release_number) const {
		const BotnetPayloadConfig& payload_config = config_.payload.interpolate_and_extrapolate(payload_release_number);
		const auto min_detecting_av_release_it = payload_config.min_detecting_av_release.find(av.get_spec());
		if (min_detecting_av_release_it != payload_config.min_detecting_av_release.end()) {
			return av.get_release_number() >= min_detecting_av_release_it->second;
		} else {
			// Jeżeli antywirus nie "wie" o botnecie, to zakładamy że nie może wykryć i leczyć infekcji.
			return false;
		}
	}
}
