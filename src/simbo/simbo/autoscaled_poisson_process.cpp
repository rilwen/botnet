#include <boost/locale.hpp>
#include "autoscaled_poisson_process.hpp"
#include "rng.hpp"

namespace simbo {
	AutoscaledPoissonProcess::AutoscaledPoissonProcess(interpolator1d_ptr&& int_lambda) {
		if (int_lambda == nullptr) {
			throw std::invalid_argument(boost::locale::translate("Null implementation provided"));
		}
		if (int_lambda->get_lower_bound() == int_lambda->get_upper_bound()) {
			throw std::invalid_argument(boost::locale::translate("Process defined on zero range"));
		}
		if (int_lambda->get_monotonicity() != Monotonicity::NOT_DECREASING) {
			throw std::invalid_argument(boost::locale::translate("Integrated intensity cannot decrease"));
		}
		int_lambda_ = std::move(int_lambda);
	}

	seconds_t AutoscaledPoissonProcess::get_next_jump(RNG& rng, const int previous_count, const seconds_t previous_time, const seconds_t horizon) const {
		check_times(previous_time, horizon);
		if (previous_count < get_min_state()) {
			throw std::domain_error(boost::locale::translate("Previous state below minimum value"));
		}		
		if (previous_count == 0) {
			// Skok niemożliwy.
			return horizon;
		}		
		const auto int_lambda_final = (*int_lambda_)(horizon) * previous_count;
		const auto int_lambda_0 = (*int_lambda_)(previous_time) * previous_count;
		const auto p_no_event = exp(int_lambda_0 - int_lambda_final);
		// P(brak zdarzeń do czasu t1|czas t0) = exp( - int_t0^t1 lambda(s) ds )
		const auto u = rng.draw_uniform();
		if (u > p_no_event) {
			// Jest przynajmniej jedno zdarzenie!
			// Czas 1-szego zdarzenia t obliczamy z równania
			// u = exp( - int_t0^t lambda(s) ds )
			// - ln(u) = int_t0^t lambda(s) ds
			// int_0^t lambda(s) ds = int_0^t0 lambda(s) ds - ln(u)
			// t0 -> t
			assert(u > 0);
			return int_lambda_->invert_right((int_lambda_0 - log(u)) / previous_count);
		} else {
			return horizon;
		}
	}
}
