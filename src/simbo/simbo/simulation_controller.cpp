#include <fstream>
#include "context.hpp"
#include "data_storage_format.hpp"
#include "exceptions.hpp"
#include "log.hpp"
#include "output_config.hpp"
#include "rng.hpp"
#include "simulation_controller.hpp"
#include "sysadmin.hpp"
#include "utils.hpp"
#include "world.hpp"

namespace simbo {

	SimulationController::SimulationController(World&& world, Context&& ctx, const OutputConfig& output_config)
		: world_(std::move(world)), ctx_(std::move(ctx)), output_config_(output_config) {
		output_config_.validate();
		data_storage_format_ = data_storage_format_from_extension(get_filename_extension(output_config.sinkhole_data_filename));
	}

	void SimulationController::step() {
		// Przedział czasowy: [ctx_.get_time(), ctx_.get_next_time()).
		// Stany obiektów są uaktualniane na początku przedziału.
		update_host_states();
		update_user_states();
		get_logger()->info("SimulationController: botnet size: {}", ctx_.get_botnet().size());
		if (ctx_.has_next_time()) {
			// Teraz dzieją się różne rzeczy w trakcie przedziału.
			const int number_emails_sent = send_infectious_emails();
			get_logger()->debug("SimulationController: number infectious emails sent: {}", number_emails_sent);			
			perform_user_actions();
		}
	}

	void SimulationController::run() {
		/// Uaktualnij zliczenia.
		get_logger()->info("SimulationController: started simulation run");
		world_.count_things();

		const auto& full_data_filename = output_config_.sinkhole_data_filename;
		std::ios_base::openmode fout_full_mode;
		if (data_storage_format_ == DataStorageFormat::CSV) {
			fout_full_mode = std::ios_base::trunc;
		} else if (data_storage_format_ == DataStorageFormat::BIN) {
			fout_full_mode = std::ios_base::trunc | std::ios::binary | std::ios::out;
		} else if (data_storage_format_ == DataStorageFormat::NONE) {
			fout_full_mode = std::ios_base::trunc;
		} else {
			throw std::invalid_argument((boost::locale::format("Unsupported data storage format {1} for filename: {2}") % data_storage_format_ % full_data_filename).str());
		}

		std::unique_ptr<std::ofstream> fout_full_ptr;
		if (data_storage_format_ != DataStorageFormat::NONE) {
			fout_full_ptr.reset(new std::ofstream(full_data_filename, fout_full_mode));
		}		

		Date current_date = ctx_.get_datetime().date();
		while (true) {
			step();
			if (ctx_.has_next_time()) {
				ctx_.make_time_step();
				get_logger()->info("SimulationController: advanced time to {}", to_string(ctx_.get_datetime()));
				if (data_storage_format_ == DataStorageFormat::BIN) {
					const Date next_date = ctx_.get_datetime().date();
					if (next_date > current_date) {
						if (fout_full_ptr) {
							ctx_.get_botnet().get_sinkhole().dump_full_bin_incremental(*fout_full_ptr);
							get_logger()->info("SimulationController: dumped sinkhole data from last day to {}", full_data_filename);
						}
						current_date = next_date;
					}
				}
			} else {
				break;
			}
		}
		get_logger()->info("SimulationController: finished simulation run");
		if (fout_full_ptr) {
			if (data_storage_format_ == DataStorageFormat::BIN) {
				ctx_.get_botnet().get_sinkhole().dump_full_bin_incremental(*fout_full_ptr);
			} else {
				ctx_.get_botnet().get_sinkhole().dump_full_csv(*fout_full_ptr);
			}
			fout_full_ptr->close();
			get_logger()->info("SimulationController: dumped sinkhole data to {}", full_data_filename);
		}
		if (!output_config_.sinkhole_reduced_data_filename.empty()) {
			std::ofstream fout_reduced(output_config_.sinkhole_reduced_data_filename, std::ios_base::trunc);
			ctx_.get_botnet().get_sinkhole().dump_reduced_csv(fout_reduced);
			fout_reduced.close();
			get_logger()->info("SimulationController: dumped reduced sinkhole data to {}", output_config_.sinkhole_reduced_data_filename);			
		}
	}

	int SimulationController::send_infectious_emails() {
		const BotnetConfig& botnet_config = ctx_.get_botnet_config();
		const int nbr_payload_releases = botnet_config.payload_release_schedule.size();		
		int number_senders = 0;
		double infectious_emails_per_second = 0;
		const DateTime today = ctx_.get_datetime();
		const double current_infectious_email_sent_per_second_by_command_centre = botnet_config.infectious_email_sent_per_second_by_command_centre.interpolate_and_extrapolate(today);
		for (int i = 0; i < nbr_payload_releases; ++i) {
			if (today >= botnet_config.payload_release_schedule.get_xs()[i]) {
				const int release_number = botnet_config.payload_release_schedule.get_ys()[i];
				const BotnetPayloadConfig& payload_config = botnet_config.payload.interpolate_and_extrapolate(release_number);
				if (payload_config.infectious_emails_sent_per_second_per_bot > 0) {
					const int number_sends_this_release = ctx_.get_botnet().calc_number_actively_propagating_bots(release_number);
					infectious_emails_per_second += payload_config.infectious_emails_sent_per_second_per_bot * number_sends_this_release;
					number_senders += number_sends_this_release;
				}				
			}
		}
		get_logger()->debug("SimulationController: number_senders: {}", number_senders);
		if (number_senders) {
			infectious_emails_per_second /= number_senders;
		}
		infectious_emails_per_second += current_infectious_email_sent_per_second_by_command_centre;
		get_logger()->debug("SimulationController: infectious_emails_per_second: {}", infectious_emails_per_second);
		
		if (infectious_emails_per_second == 0) {
			get_logger()->debug("SimulationController: exiting because no senders");
			return 0;
		}
		const seconds_t delta_time = ctx_.get_next_time() - ctx_.get_time();
		if (!(delta_time > 0)) {
			throw std::logic_error((boost::locale::format("Time interval must be positive: {1}") % delta_time).str());
		}
		
		const double total_number_emails_to_send_fractional = infectious_emails_per_second * delta_time;
		get_logger()->debug("SimulationController: total number emails to send (fractional): {}", total_number_emails_to_send_fractional);
		int total_number_emails_to_send;
		if (total_number_emails_to_send_fractional >= 1) {
			total_number_emails_to_send = static_cast<int>(infectious_emails_per_second * delta_time);
		} else {
			total_number_emails_to_send = 0;
			// W ten sposób unikamy zaokrąglania do zera.
			if (ctx_.get_rng().draw_uniform() < total_number_emails_to_send_fractional) {
				total_number_emails_to_send = 1;
			}
		}
		total_number_emails_to_send = std::min(botnet_config.email_address_list_size, total_number_emails_to_send);
		get_logger()->debug("SimulationController: total number emails to send: {}", total_number_emails_to_send);
		int number_emails_sent = 0;
		if (total_number_emails_to_send) {
			const double probability_receiving_email = static_cast<double>(total_number_emails_to_send) / static_cast<double>(botnet_config.email_address_list_size);
			get_logger()->debug("SimulationController: probability_receiving_email: {}", probability_receiving_email);
			for (auto user_it = world_.get_users_begin(); user_it != world_.get_users_end(); ++user_it) {
				const auto accounts_end = user_it->get_email_accounts_end();
				for (auto account_it = user_it->get_email_accounts_begin(); account_it != accounts_end; ++account_it) {
					if (ctx_.get_rng().draw_uniform() < probability_receiving_email) {
						const auto time = ctx_.get_time() + ctx_.get_rng().draw_uniform() * delta_time;
						account_it->receive_infectious_email(ctx_, time);
						++number_emails_sent;
					}
				}
			}
		}
		return number_emails_sent;
	}

	void SimulationController::update_host_states() {
		for (auto sysadmin_it = world_.get_sysadmins_begin(); sysadmin_it != world_.get_sysadmins_end(); ++sysadmin_it) {
			for (auto& host : sysadmin_it->get_hosts()) {
				host->update_state(ctx_);
			}
		}
	}

	void SimulationController::update_user_states() {
		for (auto user_it = world_.get_users_begin(); user_it != world_.get_users_end(); ++user_it) {
			user_it->update_state(ctx_);
		}
	}

	void SimulationController::perform_user_actions() {
		for (auto user_it = world_.get_users_begin(); user_it != world_.get_users_end(); ++user_it) {
			user_it->perform_actions(ctx_);
		}
	}
}
