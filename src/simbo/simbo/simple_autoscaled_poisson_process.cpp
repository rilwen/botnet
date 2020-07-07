#include <boost/locale.hpp>
#include "simple_autoscaled_poisson_process.hpp"
#include "rng.hpp"

namespace simbo {
	SimpleAutoscaledPoissonProcess::SimpleAutoscaledPoissonProcess(double lambda)
		: lambda_(lambda) {
		if (lambda < 0) {
			throw std::domain_error(boost::locale::translate("Negative intensity"));
		}
	}

	seconds_t SimpleAutoscaledPoissonProcess::get_next_jump(RNG& rng, const int previous_count, const seconds_t previous_time, const seconds_t horizon) const {
		if (horizon <= previous_time) {
			throw std::domain_error(boost::locale::translate("Horizon is not larger than previous time"));
		}
		// Nie trzeba sprawdzać zakresów czasu, bo proces jest określony na przedziale [-infty, infty].
		if (previous_count < get_min_state()) {
			throw std::domain_error(boost::locale::translate("Previous state below minimum value"));
		}
		if (!lambda_) {
			return horizon;
		}
		if (previous_count == 0) {
			// Skok niemożliwy.
			return horizon;
		}
		const double effective_lambda = lambda_ * previous_count;
		const auto delta_time = horizon - previous_time;
		const auto p_no_event = exp(-effective_lambda * delta_time);
		// P(brak zdarzeń do czasu t1|czas t0) = exp( - (t1 - t0) * effective_lambda )
		const auto u = rng.draw_uniform();
		if (u > p_no_event) {
			// Jest przynajmniej jedno zdarzenie!
			// Czas 1-szego zdarzenia t obliczamy z równania
			// u = exp( - (t - t0) * effective_lambda )
			// - ln(u) = (t - t0) * effective_lambda
			// t = t0 - ln(u) / effective_lambda
			assert(u > 0);
			return previous_time - log(u) / effective_lambda;
		} else {
			return horizon;
		}
	}
}
