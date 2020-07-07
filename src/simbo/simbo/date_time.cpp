#include "json.hpp"
#include "date_time.hpp"

namespace simbo {
	TimeDuration time_duration_from_seconds(const seconds_t seconds) {
		if (seconds < 0) {
			return -time_duration_from_seconds(-seconds);
		}
		const auto total_seconds = std::floor(seconds);
		const auto fractional_seconds = seconds - total_seconds;
		return boost::posix_time::seconds(1) * total_seconds + TimeDuration(0, 0, 0, fractional_seconds);
	}

	bool is_weekend(const Date& date) {
		const auto day_of_week = date.day_of_week();
		return day_of_week == 6 || day_of_week == 0; // Wg. Boost, sobota == 6, niedziela == 0.
	}

	seconds_t get_seconds_from_json(const json& j) {
		if (j.is_number()) {
			return j.get<seconds_t>();
		} else if (j.is_object()) {
			TimeDuration td = j;
			return to_seconds(td);
		} else {
			throw DeserialisationError("TimeDuration", j.dump(), "Expected a number of seconds or TimeDuration");
		}
	}
}

namespace boost {
	namespace gregorian {
		void to_json(json& j, const boost::gregorian::date& d) {
			j = simbo::to_string(d);
		}

		void from_json(const json& j, boost::gregorian::date& d) {
			if (j.is_object()) {
				simbo::validate_keys(j, "Date", { "year", "month", "day" }, {});
				try {
					const unsigned short year = j["year"];
					const unsigned short month = j["month"];
					const unsigned short day = j["day"];
					d = boost::gregorian::date(year, month, day);
				} catch (std::exception& e) {
					throw simbo::DeserialisationError("Date", j.dump(), e);
				}
			} else if (j.is_string()) {
				try {
					d = from_string(j.get<std::string>());
				} catch (std::exception& e) {
					throw simbo::DeserialisationError("Date", j.dump(), e);
				}
			} else {
				throw simbo::DeserialisationError("Date", j.dump(), boost::locale::translate("JSON data must be a string or an object"));
			}
		}
	}
	namespace posix_time {
		void to_json(json& j, const boost::posix_time::time_duration& td) {
			j = json{
				{"hours", td.hours()},
				{"minutes", td.minutes()},
				{"seconds", td.seconds()},
				{"microseconds", td.fractional_seconds()}
			};
		}

		void from_json(const json& j, boost::posix_time::time_duration& td) {
			simbo::validate_keys(j, "TimeDuration", {}, { "hours", "minutes", "seconds", "microseconds" });
			try {
				const long hours = simbo::get_value(j, "hours", 0);
				const long mins = simbo::get_value(j, "minutes", 0);
				const long secs = simbo::get_value(j, "seconds", 0);
				const long usecs = simbo::get_value(j, "microseconds", 0);
				td = boost::posix_time::time_duration(hours, mins, secs, usecs);
			} catch (std::exception& e) {
				throw simbo::DeserialisationError("TimeDuration", j.dump(), e);
			}
		}

		void to_json(json& j, const boost::posix_time::ptime& ptime) {
			j = json{
				{"date", ptime.date()},
				{"time", ptime.time_of_day()}
			};
		}

		void from_json(const json& j, boost::posix_time::ptime& ptime) {
			if (j.is_object()) {
				simbo::validate_keys(j, "DateTime", { "date" }, { "time" });
				try {
					boost::gregorian::date date = j["date"];
					boost::posix_time::time_duration time = simbo::get_value(j, "time", boost::posix_time::time_duration(0, 0, 0, 0));
					ptime = boost::posix_time::ptime(date, time);
				} catch (std::exception& e) {
					throw simbo::DeserialisationError("DateTime", j.dump(), e);
				}
			} else if (j.is_string()) {
				// Tylko data.
				try {
					boost::gregorian::date date;
					from_json(j, date);
					boost::posix_time::time_duration time(0, 0, 0, 0);
					ptime = boost::posix_time::ptime(date, time);
				} catch (std::exception& e) {
					throw simbo::DeserialisationError("DateTime", j.dump(), e);
				}
			} else {
				throw simbo::DeserialisationError("DateTime", j.dump(), boost::locale::translate("JSON data must be a string or an object"));
			}
		}

		std::ostream& operator<<(std::ostream& os, const boost::posix_time::ptime& ptime) {
			os << boost::posix_time::to_iso_extended_string(ptime);
			return os;
		}
	}
}
