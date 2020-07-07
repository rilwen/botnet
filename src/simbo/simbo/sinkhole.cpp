#include <cassert>
#include <ostream>
#include <boost/format.hpp>
#include "exceptions.hpp"
#include "host.hpp"
#include "local_network.hpp"
#include "log.hpp"
#include "public_connection.hpp"
#include "sinkhole.hpp"

namespace simbo {

	const DateTime Sinkhole::EPOCH(Date(1970, 1, 1), TimeDuration(0, 0, 0));

	Sinkhole::Sinkhole(DateTime simulation_start, bool save_full_data)
		: nbr_data_points_dumped_(0),
		simulation_start_(simulation_start),
		save_full_data_(save_full_data),
		accumulation_date_(simulation_start.date()),
		accumulation_hour_(simulation_start.time_of_day().hours())
	{}

	Sinkhole::data_point_t Sinkhole::get_data_from_bot(const Host* host, const seconds_t time) const {
		if (!host) {
			throw std::invalid_argument(boost::locale::translate("Null host"));
		}
		if (!host->has_internet_access()) {
			throw std::invalid_argument(boost::locale::translate("Host does not have internet access"));
		}
		if (!host->has_botnet_role()) {
			throw std::invalid_argument(boost::locale::translate("Host does not have any botnet role"));
		}
		const LocalNetwork* const local_network = host->get_local_network();
		assert(local_network);
		seconds_t time_from_epoch;
		if (EPOCH != simulation_start_) {
			const DateTime date_time = simulation_start_ + time_duration_from_seconds(time);
			time_from_epoch = to_seconds(date_time - EPOCH);
		} else {
			time_from_epoch = time;
		}
		return data_point_t(
			time_from_epoch,
			local_network->get_public_connection()->get_ip_address(),
			host->get_id(),
			host->get_bot_state().get_value(),
			local_network->get_type(),
			host->is_fixed(),
			local_network->get_country(),
			host->get_payload_release_number()
		);
	}

	void Sinkhole::add_data_point(data_point_t&& new_data_point) {
		// Dodany właśnie punkt danych.
		const data_point_t *added_data_point_ptr;
		if (save_full_data_) {
			data_.push_back(std::move(new_data_point));
			added_data_point_ptr = &data_.back();
		} else {
			added_data_point_ptr = &new_data_point;
		}

		const DateTime datetime = EPOCH + time_duration_from_seconds(added_data_point_ptr->time);
		const Date new_date = datetime.date();
		const int new_hour = datetime.time_of_day().hours();
		if ((new_date > accumulation_date_) || (new_hour > accumulation_hour_)) {
			global_reduced_data_.push_back(GlobalReducedDataWithTime(global_reduced_hourly_data_accumulator_, accumulation_date_, accumulation_hour_));
			reset_global_reduced_data_accumulator(global_reduced_hourly_data_accumulator_);
		}
		global_reduced_hourly_data_accumulator_[added_data_point_ptr->country].add_data_point(added_data_point_ptr->ip, added_data_point_ptr->host_id);
		if (new_date > accumulation_date_) {
			global_reduced_data_.push_back(GlobalReducedDataWithTime(global_reduced_daily_data_accumulator_, accumulation_date_, HOUR_FOR_DAILY_DATA));
			reset_global_reduced_data_accumulator(global_reduced_daily_data_accumulator_);
		}
		global_reduced_daily_data_accumulator_[added_data_point_ptr->country].add_data_point(added_data_point_ptr->ip, added_data_point_ptr->host_id);
		accumulation_date_ = new_date;
		accumulation_hour_ = new_hour;
	}

	void Sinkhole::dump_full_csv(std::ostream& out) const {
		out << "time,bot_state,ip,host_id,local_network_type,is_fixed,country\n";
		for (const auto& row : data_) {
			// Casty do int służą poprawnemu formatowaniu tekstu w pliku CSV.
			out << row.time << "," << static_cast<int>(row.bot_state) << "," << row.ip << "," << row.host_id << "," << static_cast<int>(row.local_network_type) << "," << static_cast<int>(row.is_fixed) << "," << static_cast<int>(row.country) << '\n';
		}
	}	

	void Sinkhole::dump_reduced_csv(std::ostream& out) const {
		out << "date,hour,country,num_ip_addresses,num_messages,num_hosts\n"; // Czas jest zaokrąglony do 1h.
		for (const GlobalReducedDataWithTime& global_rows : global_reduced_data_) {
			write_global_reduced_rows_csv(out, global_rows);
		}
		write_global_reduced_rows_csv(out, GlobalReducedDataWithTime(global_reduced_hourly_data_accumulator_, accumulation_date_, accumulation_hour_));
		write_global_reduced_rows_csv(out, GlobalReducedDataWithTime(global_reduced_daily_data_accumulator_, accumulation_date_, HOUR_FOR_DAILY_DATA));		
	}

	// Sprawdź czy architektura maszyny jest little-endian.
	static bool is_little_endian() {
		int16_t num = 1;
		return *(int8_t*)&num == 1;
	}

	void Sinkhole::write_header(size_t number_rows, std::ostream& out) {
		Header header(number_rows);
		out.write((char*)&header, sizeof(Header));
	}

	void Sinkhole::dump_full_bin(std::ostream& out) const {
		if (!is_little_endian()) {
			throw std::runtime_error(boost::locale::translate("Only little endian machines supported"));
		}
		const size_t n_rows = data_.size();
		write_header(n_rows, out);
		if (n_rows) {
			// Zapisz dane.
			out.write((char*)&data_[0], sizeof(data_point_t) * n_rows);
		}
	}

	void Sinkhole::wipe_full_data() {
		data_.clear();
		data_.shrink_to_fit();
	}

	void Sinkhole::dump_full_bin_incremental(std::ostream& out) {
		if (!is_little_endian()) {
			throw std::runtime_error(boost::locale::translate("Only little endian machines supported"));
		}
		const std::streampos current_out_pos = out.tellp();
		if (current_out_pos == static_cast<std::streampos>(-1)) {
			throw std::runtime_error(boost::locale::translate("Error while getting current position in binary output stream"));
		}
		const bool first_write = current_out_pos == static_cast<std::streampos>(0);
		const size_t n_rows = data_.size();
		if (first_write) {
			if (nbr_data_points_dumped_) {
				throw std::logic_error(boost::locale::translate("Output stream state does not match sinkhole state"));
			}
			dump_full_bin(out);
		} else {			
			if (n_rows) {
				// Zapisz nagłówek.
				out.seekp(0);
				write_header(n_rows + nbr_data_points_dumped_, out);
				// Zapisz dane.
				out.seekp(current_out_pos);
				out.write((char*)&data_[0], sizeof(data_point_t) * n_rows);
			}
		}
		wipe_full_data();
		nbr_data_points_dumped_ += n_rows;
	}

	void Sinkhole::write_global_reduced_rows_csv(std::ostream& out, const GlobalReducedDataWithTime& reduced_data_row) const {
		int country_index = 0;
		const auto date = reduced_data_row.date;
		const auto hour = reduced_data_row.hour;
		for (const auto& country_data : reduced_data_row.data) {
			if (country_data.message_count) {
				out << date << "," << hour << "," << country_index << "," << country_data.ip_address_count << "," << country_data.message_count << "," << country_data.host_count << "\n";
			}
			++country_index;
		}
	}

	Sinkhole::Header::Header(size_t n_number_rows)
		: number_rows(static_cast<uint64_t>(n_number_rows)) {
		std::fill(reserved, reserved + reserved_size, 0);
	}

	Sinkhole::ReducedDataAccumulator::ReducedDataAccumulator()
		: message_count_(0) {
	}

	void Sinkhole::ReducedDataAccumulator::add_data_point(data_point_t::ip_t ip_address, data_point_t::host_id_t host_id) {
		ip_addresses_.insert(ip_address);
		host_ids_.insert(host_id);
		++message_count_;
	}

	void Sinkhole::ReducedDataAccumulator::reset() {
		ip_addresses_.clear();
		host_ids_.clear();
		message_count_ = 0;
	}

	Sinkhole::ReducedData::ReducedData() :
		ip_address_count(0), host_count(0), message_count(0) 
	{}

	Sinkhole::ReducedData& Sinkhole::ReducedData::operator=(const ReducedDataAccumulator& accumulator) {
		ip_address_count = static_cast<int64_t>(accumulator.get_ip_addresses().size());
		host_count = static_cast<int64_t>(accumulator.get_host_ids().size());
		message_count = accumulator.get_message_count();
		return *this;
	}

	Sinkhole::GlobalReducedDataWithTime::GlobalReducedDataWithTime(const global_reduced_data_accumulator_t& accumulator, const Date& acc_date, int acc_hour) :
		date(acc_date), hour(acc_hour) {
		int64_t nbr_messages = 0;
		for (size_t i = 0; i < data.size(); ++i) {
			data[i] = accumulator[i];
			nbr_messages += data[i].message_count;
		}
		get_logger()->debug((boost::format("Reduced data from accumulator for day %s, hour %d: %d messages in total") % acc_date % acc_hour % nbr_messages).str());
	}

	void Sinkhole::reset_global_reduced_data_accumulator(global_reduced_data_accumulator_t& global_reduced_data_accumulator) {
		for (auto& acc : global_reduced_data_accumulator) {
			acc.reset();
		}
	}
}
