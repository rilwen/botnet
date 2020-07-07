#include "exceptions.hpp"
#include "own_time_counter.hpp"

namespace simbo {
	OwnTimeCounter::OwnTimeCounter(seconds_t init_wall_time)
		: own_time_(0),
		last_update_wall_time_(init_wall_time) {
	}

	void OwnTimeCounter::reset(const seconds_t wall_time) {
		if (wall_time >= last_update_wall_time_) {
			own_time_ = 0;
			last_update_wall_time_ = wall_time;
		} else {
			// Zresetuj czas własny "wstecz", jeżeli jest to kompatybilne z historią.
			const seconds_t how_far_back = last_update_wall_time_ - wall_time;
			if (own_time_ >= how_far_back) {
				// Można to zrobić.
				own_time_ = how_far_back;
			} else {
				throw (std::domain_error(
					(boost::locale::format(boost::locale::translate("OwnTimeCounter: Cannot reset own time because reset wall time {} is before last update wall time {} and there is not enough own time {} to allow for reset-in-the-past compatible with history")) % wall_time % last_update_wall_time_ % own_time_).str()));
			}
		}		
	}

	void OwnTimeCounter::update(const seconds_t wall_time, bool own_time_was_running) {
		if (wall_time < last_update_wall_time_) {
			throw (std::domain_error(
				(boost::locale::format(boost::locale::translate("OwnTimeCounter: Wall time {1} of the update of own time is before last update wall time {2}")) % wall_time % last_update_wall_time_).str()));
		}
		if (own_time_was_running) {
			own_time_ += wall_time - last_update_wall_time_;
		}
		last_update_wall_time_ = wall_time;
	}
}
