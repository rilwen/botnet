#include <iostream>
#include "context.hpp"
#include "exceptions.hpp"
#include "host.hpp"
#include "log.hpp"
#include "rng.hpp"
#include "user.hpp"

namespace simbo {
	User::User(host_switcher_t&& host_switcher, email_accounts_vec&& email_accounts, double probability_email_activated,
		double initial_email_opening_probability, double email_opening_probability_multiplier_exponent,
		double probability_instant_email_check)
		: host_switcher_(std::move(host_switcher)),
		email_accounts_(std::move(email_accounts)) {
		set_initial_email_opening_probability(initial_email_opening_probability);
		set_email_opening_probability_multiplier_exponent(email_opening_probability_multiplier_exponent);
		set_probability_email_activated(probability_email_activated);
		if (!(probability_instant_email_check >= 0 && probability_instant_email_check <= 1)) {
			throw std::domain_error(boost::locale::translate("Probability of instant email check outside [0, 1]"));
		}
		probability_instant_email_check_ = probability_instant_email_check;
		set_email_account_monitoring();
	}

	User::User(User&& other)
		: host_switcher_(std::move(other.host_switcher_)),
		email_accounts_(std::move(other.email_accounts_)),
		probability_email_activated_(other.probability_email_activated_),
		initial_email_opening_probability_(other.initial_email_opening_probability_),
		email_opening_probability_multiplier_exponent_(other.email_opening_probability_multiplier_exponent_),
		probability_instant_email_check_(other.probability_instant_email_check_) {
		set_email_account_monitoring();
		other.wipe();
	}

	void User::update_used_host(Context& ctx) {
		host_ptr old_host = host_switcher_.get_current_value();
		host_ptr new_host = host_switcher_.update(ctx.get_rng(), ctx.get_time());
		if (old_host != new_host) {
			if (old_host) {
				assert(old_host->get_user_counter() > 0);
				old_host->remove_user();
				if (old_host->get_user_counter() == 0 && old_host->is_on()) {
					get_logger()->debug("User: turning OFF host {} at time {}", old_host->get_id(), ctx.get_time());
					old_host->turn_off(ctx, ctx.get_time());
				}
			}
			if (new_host) {
				if (!new_host->is_on()) {
					get_logger()->debug("User: turning ON host {} at time {}", new_host->get_id(), ctx.get_time());
					new_host->turn_on(ctx, ctx.get_time());
				}
				new_host->add_user();
			}
		}
	}

	void User::set_probability_email_activated(double p) {
		if (!(p >= 0 && p <= 1)) {
			throw std::domain_error(boost::locale::translate("Probability of activating opened email outside [0, 1]"));
		}
		probability_email_activated_ = p;
	}

	void User::set_initial_email_opening_probability(double p) {
		if (!(p >= 0 && p <= 1)) {
			throw std::domain_error(boost::locale::translate("Initial email opening probability outside [0, 1]"));
		}
		initial_email_opening_probability_ = p;
	}

	void User::set_email_opening_probability_multiplier_exponent(double k) {
		if (!(k >= 0)) {
			throw std::domain_error(boost::locale::translate("Exponent value must be non-negative"));
		}
		email_opening_probability_multiplier_exponent_ = k;
	}

	bool User::has_access_to_internet() const {
		const host_ptr used_host = get_used_host();
		return used_host != nullptr && used_host->is_on() && used_host->has_internet_access();
	}

	bool User::check_email_account(Context& ctx, EmailAccount& email_account, seconds_t check_time) {
		if (!has_access_to_internet()) {
			return false;
		}
		const host_ptr used_host = get_used_host();
		assert(used_host);
		if (email_account.is_host_allowed(used_host)) {
			if (is_time_undefined(check_time)) {
				check_time = email_account.email_check_time(ctx);
			}
			if (is_time_undefined(check_time)) {
				// Użytkownik nie sprawdzi poczty na tym koncie w tym przedziale czasu.
				return false;
			}
			if (check_time <= email_account.get_last_check_time()) {
				get_logger()->debug("User: not checking email on host {} at time {} because it will have been checked at time {}", used_host->get_id(), check_time, email_account.get_last_check_time());
				return false;
			}
			assert(ctx.is_inside_current_time_step(check_time));
			get_logger()->debug("User: checking email on host {} at time {}", used_host->get_id(), check_time);
			const int initial_number_unopened_emails = email_account.get_number_unopened_infectious_emails();
			int number_opened_emails = 0; // Liczba emaili otwartych w tej sesji.				
			for (int i = 0; i < initial_number_unopened_emails; ++i) {
				const int number_sessions_unopened = email_account.get_number_sessions_email_unopened(i - number_opened_emails);
				const double p = initial_email_opening_probability_ / std::pow(static_cast<double>(number_sessions_unopened + 1), email_opening_probability_multiplier_exponent_);
				if (ctx.get_rng().draw_uniform() < p) {
					email_account.open_infectious_email(i - number_opened_emails);
					++number_opened_emails;
					if (ctx.get_rng().draw_uniform() < probability_email_activated_) {
						// Użytkownik aktywował zainfekowany email, np. klikając na załącznik.						
						used_host->try_infecting_via_email(ctx, check_time);
					}
				} else {
					email_account.skip_infectious_email(i - number_opened_emails);
				}
			}
			email_account.set_last_check_time(check_time);
			return true;
		} else {
			return false;
		}
	}

	int User::check_email(Context& ctx) {		
		if (!has_access_to_internet()) {
			// Użytkownik musi używać włączonego komputera z dostępem do Internetu, żeby sprawdzić pocztę.			
			return 0;
		}
		int number_accounts_checked = 0;
		for (EmailAccount& email_account : email_accounts_) {
			number_accounts_checked += static_cast<int>(check_email_account(ctx, email_account, undefined_time()));
		}
		return number_accounts_checked;
	}

	void User::update_state(Context& ctx) {
		update_used_host(ctx);
	}

	void User::perform_actions(Context& ctx) {
		check_email(ctx);
	}

	void User::wipe() {
		email_accounts_.clear();
		probability_email_activated_ = 0;
		initial_email_opening_probability_ = 0;
		email_opening_probability_multiplier_exponent_ = 1;
		probability_instant_email_check_ = 0;
	}

	void User::set_email_account_monitoring() {
		if (probability_instant_email_check_ > 0) {
			for (EmailAccount& email_account : email_accounts_) {
				email_account.set_monitoring_user(this);
			}
		}
	}
}
