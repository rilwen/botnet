#include "context.hpp"
#include "email_account.hpp"
#include "email_provider.hpp"
#include "exceptions.hpp"
#include "host.hpp"
#include "jump_markov_process.hpp"
#include "log.hpp"
#include "rng.hpp"
#include "user.hpp"

namespace simbo {
	EmailAccount::EmailAccount(email_provider_ptr email_provider, check_process_ptr check_process, hosts_set&& allowed_hosts, local_networks_set&& allowed_networks)
		: email_provider_(email_provider), check_process_(check_process), monitoring_user_(nullptr) {
		if (!email_provider) {
			throw std::invalid_argument(boost::locale::translate("Email provider is null"));
		}
		if (!check_process) {
			throw std::invalid_argument(boost::locale::translate("Check process is null"));
		}
		if (std::any_of(allowed_hosts.begin(), allowed_hosts.end(), [](host_ptr host) {
			return !host;
		})) {
			throw std::invalid_argument(boost::locale::translate("Allowed host is null"));
		}
		if (std::any_of(allowed_networks.begin(), allowed_networks.end(), [](local_network_ptr local_network) {
			return !local_network;
		})) {
			throw std::invalid_argument(boost::locale::translate("Allowed local network is null"));
		}
		allowed_hosts_ = std::move(allowed_hosts);
		allowed_networks_ = std::move(allowed_networks);
	}

	bool EmailAccount::is_host_allowed(host_ptr host) const {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Host is null"));
		}
		if (!host->get_local_network()) {
			throw std::invalid_argument(boost::locale::translate("Local network is null"));
		}
		bool host_ok;
		if (allowed_hosts_.empty()) {
			host_ok = true;
		} else {
			host_ok = allowed_hosts_.count(host) != 0;
		}
		return host_ok && is_network_allowed(host->get_local_network());
	}

	seconds_t EmailAccount::email_check_time(Context& ctx) const {
		const seconds_t to = ctx.get_next_time();
		assert(ctx.get_time() < to);
		const seconds_t jump_time = check_process_->get_next_jump(ctx.get_rng(), 0, ctx.get_time(), to);
		if (jump_time < to) {
			return jump_time;
		} else {
			return undefined_time();
		}
	}

	void EmailAccount::receive_infectious_email(Context& ctx, const seconds_t time) {
		if (!ctx.is_inside_current_time_step(time)) {
			throw std::domain_error(boost::locale::translate("Email receiving time outside current schedule period"));
		}
		if (ctx.get_rng().draw_uniform() >= email_provider_->get_email_filtering_efficiency()) {
			unopened_infectious_emails_.push_back(std::make_pair(0, time));			
			get_logger()->debug("EmailAccount: Received infectious email at time {}", time);
		}
		if (monitoring_user_ && ctx.get_rng().draw_uniform() < monitoring_user_->get_probability_instant_email_check()) {
			get_logger()->debug("EmailAccount: Doing instantenous email check at time {}", time);
			monitoring_user_->check_email_account(ctx, *this, time);
		}
	}

	void EmailAccount::open_infectious_email(const int email_index) {
		if (email_index < 0 || email_index >= get_number_unopened_infectious_emails()) {
			throw std::out_of_range(boost::locale::translate("Infectious email index out of range"));
		}
		unopened_infectious_emails_.erase(unopened_infectious_emails_.begin() + email_index);
	}

	void EmailAccount::skip_infectious_email(const int email_index) {
		if (email_index < 0 || email_index >= get_number_unopened_infectious_emails()) {
			throw std::out_of_range(boost::locale::translate("Infectious email index out of range"));
		}
		++unopened_infectious_emails_[email_index].first;
	}

	int EmailAccount::get_number_sessions_email_unopened(int email_index) const {
		if (email_index < 0 || email_index >= get_number_unopened_infectious_emails()) {
			throw std::out_of_range(boost::locale::translate("Infectious email index out of range"));
		}
		return unopened_infectious_emails_[email_index].first;
	}

	bool EmailAccount::is_network_allowed(local_network_ptr local_network) const {
		if (allowed_networks_.empty()) {
			return true;
		} else {
			return allowed_networks_.count(local_network) != 0;
		}
	}

	void EmailAccount::set_monitoring_user(User* user) {
		monitoring_user_ = user;
	}

	void EmailAccount::set_last_check_time(seconds_t time) {
		if (time < last_check_time_) {
			throw std::domain_error((boost::locale::format("EmailAccount: new last check time {1} before the previous one {2}") % time % last_check_time_).str());
		}
		last_check_time_ = time;
	}
}
