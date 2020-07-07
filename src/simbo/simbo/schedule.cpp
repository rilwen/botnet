#include <stdexcept>
#include <boost/locale.hpp>
#include "json.hpp"
#include "schedule.hpp"

namespace simbo {
	Schedule::Schedule(DateTime start)
		: points_({ start }), intervals_from_start_({ 0. }), size_(1) {

	}

	Schedule::Schedule(const DateTime start, const TimeDuration period, const int nbr_points)
		: points_(nbr_points), intervals_from_start_(nbr_points), size_(nbr_points) {
		if (period.total_microseconds() <= 0) {
			throw std::invalid_argument(boost::locale::translate("Schedule period must be positive"));
		}
		if (nbr_points <= 0) {
			throw std::domain_error(boost::locale::translate("Number of time points must be positive"));
		}
		auto date_time = start;
		for (int i = 0; i < nbr_points; ++i) {
			points_[i] = date_time;
			intervals_from_start_[i] = to_seconds(date_time - start);
			date_time += period;
		}
	}

	Schedule::Schedule(std::vector<seconds_t>&& intervals_from_start)
		: intervals_from_start_(std::move(intervals_from_start)) {
		if (intervals_from_start_.empty() || intervals_from_start_.front() != seconds_t(0)) {
			intervals_from_start_.insert(intervals_from_start_.begin(), seconds_t(0));
		}
		size_ = static_cast<int>(intervals_from_start_.size());
		points_.reserve(size_);
		const DateTime start(Date(1970, 1, 1), TimeDuration(0, 0, 0, 0));
		seconds_t previous_interval = -1;
		for (auto interval : intervals_from_start_) {
			if (interval <= previous_interval) {
				throw std::invalid_argument(boost::locale::translate("Intervals are not strictly increasing"));
			}
			const auto point = start + time_duration_from_seconds(interval);
			points_.push_back(point);
			previous_interval = interval;
		}
	}

	Schedule::Schedule(std::vector<DateTime>&& points)
		: size_(static_cast<int>(points.size())) {
		if (points.empty()) {
			throw std::invalid_argument(boost::locale::translate("Empty points vector"));
		}
		intervals_from_start_.reserve(size_);
		intervals_from_start_.push_back(0);
		auto prev_datetime = points.front();
		for (int i = 1; i < size_; ++i) {
			const auto& datetime = points[i];
			if (!(datetime > prev_datetime)) {
				throw std::invalid_argument((boost::locale::format("DateTime {1} not past the previous DateTime {2}") % datetime % prev_datetime).str());
			}
			intervals_from_start_.push_back(to_seconds(datetime - points.front()));
		}
		points_ = std::move(points);
	}

	Schedule& Schedule::operator=(Schedule&& other) {
		if (this != &other) {
			points_ = std::move(other.points_);
			intervals_from_start_ = std::move(other.intervals_from_start_);
			size_ = other.size_;
			other.wipe();
		}
		return *this;
	}

	static void validate_offsets(const std::vector<TimeDuration>& offsets) {
		// Sprawdź czy przesunięcia czasowe mieszczą się w tym samym dniu.
		const size_t n = offsets.size();
		seconds_t previous_offset_in_sec = -1;
		for (size_t i = 0; i < n; ++i) {
			const seconds_t offset_in_sec = to_seconds(offsets[i]);
			if (!(offset_in_sec >= 0)) {
				throw std::domain_error((boost::locale::format("Offset {1} is negative") % offsets[i]).str());
			}
			if (!(offset_in_sec < 24 * 3600)) {
				throw std::domain_error((boost::locale::format("Offset {1} should be be less than 24h") % offsets[i]).str());
			}
			if (!(offset_in_sec > previous_offset_in_sec)) {
				assert(i > 0);
				throw std::invalid_argument((boost::locale::format("Offset {1} is not past the preceding offset {2}") % offsets[i] % offsets[i - 1]).str());
			}
			previous_offset_in_sec = offset_in_sec;
		}
	}

	static std::vector<DateTime> make_points_weekly_seasonal(Date start_date, const std::vector<TimeDuration>& workday_offsets, const std::vector<TimeDuration>& weekend_offsets, const int nbr_days, const bool complete_last_day) {
		if (nbr_days < 1) {
			throw std::domain_error((boost::locale::format("Number of days should be >= 1, but is {1}") % nbr_days).str());
		}
		validate_offsets(workday_offsets);
		validate_offsets(weekend_offsets);
		std::vector<DateTime> points;
		const auto one_day = boost::gregorian::days(1);
		Date date = start_date;
		for (int i = 0; i < nbr_days; ++i) {
			const std::vector<TimeDuration>& offsets = is_weekend(date) ? weekend_offsets : workday_offsets;
			const size_t n = offsets.size();
			for (size_t k = 0; k < n; ++k) {
				points.push_back(DateTime(date, offsets[k]));
			}
			date += one_day;
		}
		if (complete_last_day) {
			// Dodaj ostatni punkt następnego dnia o 00:00
			points.push_back(DateTime(start_date + boost::gregorian::days(nbr_days), TimeDuration()));
		}
		points.shrink_to_fit();
		return points;
	}

	Schedule::Schedule(Date start_date, const std::vector<TimeDuration>& workday_offsets, const std::vector<TimeDuration>& weekend_offsets, int nbr_days, bool complete_last_day)
		: Schedule(make_points_weekly_seasonal(start_date, workday_offsets, weekend_offsets, nbr_days, complete_last_day)) {}

	Schedule::Schedule(Schedule&& other)
		: points_(std::move(other.points_)),
		intervals_from_start_(std::move(other.intervals_from_start_)),
		size_(other.size_) {
		other.wipe();
	}

	seconds_t Schedule::get_interval_seconds(const int i1, const int i2) const {
		assert(i1 < size());
		assert(i2 < size());
		return intervals_from_start_[i2] - intervals_from_start_[i1];
	}	

	void Schedule::wipe() {
		points_.clear();
		intervals_from_start_.clear();
		size_ = 0;
	}
}

namespace nlohmann {
	simbo::Schedule adl_serializer<simbo::Schedule>::from_json(const json& j) {
		simbo::validate_keys(j, "Schedule", {"start", "period", "nbr_points"}, {});
		try {
			const simbo::DateTime start = j["start"];
			const simbo::TimeDuration period = j["period"];
			const int nbr_points = j["nbr_points"];
			return simbo::Schedule(start, period, nbr_points);
		} catch (std::exception& e) {
			throw simbo::DeserialisationError("Schedule", j.dump(), e);
		}
	}
}
