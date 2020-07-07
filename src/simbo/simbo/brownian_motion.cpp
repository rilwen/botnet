#include <boost/locale.hpp>
#include "brownian_motion.hpp"
#include "rng.hpp"

namespace simbo {
	BrownianMotion::BrownianMotion(interpolator1d_ptr&& int_volatility_squared, interpolator1d_ptr&& int_drift) {
		if (int_volatility_squared == nullptr || int_drift == nullptr) {
			throw std::invalid_argument(boost::locale::translate("Null implementation provided"));
		}
		if (int_drift->get_upper_bound() <= int_volatility_squared->get_lower_bound() || int_volatility_squared->get_upper_bound() <= int_drift->get_lower_bound()) {
			throw std::invalid_argument(boost::locale::translate("Drift and volatility domains do not overlap on a non-zero range"));
		}
		int_vol_sqr_ = std::move(int_volatility_squared);
		int_drift_ = std::move(int_drift);
		first_time_ = std::max(int_drift_->get_lower_bound(), int_vol_sqr_->get_lower_bound());
		last_time_ = std::min(int_drift_->get_upper_bound(), int_vol_sqr_->get_upper_bound());
	}

	double BrownianMotion::get_next_state(RNG& rng, const double& previous_state, seconds_t previous_time, seconds_t next_time) const {
		check_times(previous_time, next_time);
		const auto variance = (*int_vol_sqr_)(next_time) - (*int_vol_sqr_)(previous_time);
		const auto mean = (*int_drift_)(next_time) - (*int_drift_)(previous_time);
		const auto std_deviation = std::sqrt(std::max(variance, 0.)); // W razie numerycznych błędów.
		const auto epsilon = rng.draw_gaussian();
		return previous_state + mean + epsilon * std_deviation;
	}

	double BrownianMotion::instant_switch(RNG&, const double& other_process_state, seconds_t time) const {
		check_time(time);
		return other_process_state;
	}
}
