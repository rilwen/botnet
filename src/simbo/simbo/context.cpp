#include "context.hpp"
#include "exceptions.hpp"

namespace simbo {
	Context::Context()
		: schedule_(DateTime(Date(1970, 1, 1), TimeDuration(0, 0, 0, 0))),
		time_point_index_(0), current_time_(0.), next_time_(undefined_time()) {}

	Context::Context(std::shared_ptr<RNG> rng, std::unique_ptr<Botnet>&& botnet, Schedule&& schedule)
		: rng_(rng), 
		botnet_(std::move(botnet)),
		schedule_(std::move(schedule)),
		time_point_index_(0), current_time_(0.) {
		if (!rng) {
			throw std::invalid_argument(boost::locale::translate("Null random number generator"));
		}
		if (!botnet_) {
			throw std::invalid_argument(boost::locale::translate("Null botnet"));
		}
		next_time_ = get_next_time(0);
	}

	Context::Context(Context&& other)
		: rng_(other.rng_),
		botnet_(std::move(other.botnet_)),
		schedule_(std::move(other.schedule_)),
		time_point_index_(other.time_point_index_),
		current_time_(other.current_time_),
		next_time_(other.next_time_) {
		other.wipe();
	}

	Context& Context::operator=(Context&& other) {
		if (this != &other) {
			rng_ = other.rng_;
			botnet_ = std::move(other.botnet_);
			schedule_ = std::move(other.schedule_);
			time_point_index_ = other.time_point_index_;
			current_time_ = other.current_time_;
			other.wipe();
		}
		return *this;
	}

	const BotnetConfig& Context::get_botnet_config() const {
		assert(botnet_);
		return botnet_->get_config();
	}

	void Context::make_time_step() {
		const int new_index = time_point_index_ + 1;
		if (new_index == get_number_time_points()) {
			throw std::logic_error(boost::locale::translate("No more schedule time points"));
		}
		time_point_index_ = new_index;
		current_time_ = schedule_.get_interval_from_start(new_index);
		next_time_ = get_next_time(new_index);
	}

	seconds_t Context::get_next_time(const int time_point_index) const {
		if (time_point_index + 1 == schedule_.size()) {
			return undefined_time();
		} else {
			return schedule_.get_interval_from_start(time_point_index + 1);
		}
	}

	bool Context::is_inside_current_time_step(seconds_t time) const {
		if (time_point_index_ + 1 == schedule_.size()) {
			return time == get_time();
		} else {
			return time >= get_time() && time < get_next_time();
		}
	}

	DateTime Context::get_datetime() const {
		return schedule_.get_point(time_point_index_);
	}

	void Context::wipe() {
		rng_ = nullptr;
		time_point_index_ = -1;
		current_time_ = undefined_time();
		next_time_ = undefined_time();
	}
}
