#include "exceptions.hpp"
#include "json.hpp"
#include "rng.hpp"
#include "simple_poisson_process.hpp"

namespace simbo {
	SimplePoissonProcess::SimplePoissonProcess(double lambda)
		: lambda_(lambda) {
		if (lambda < 0) {
			throw std::domain_error(boost::locale::translate("Negative intensity"));
		}
	}

	seconds_t SimplePoissonProcess::get_next_jump(RNG& rng, const int previous_count, const seconds_t previous_time, const seconds_t horizon) const {
		if (horizon <= previous_time) {
			throw std::domain_error(boost::locale::translate("Horizon is not larger than previous time"));
		}
		if (!lambda_) {
			return horizon;
		}
		// Nie trzeba sprawdzać zakresów czasu, bo proces jest określony na przedziale [-infty, infty].
		assert(previous_count >= get_min_state()); // Ponieważ get_min_state() == INT_MIN
		const auto delta_time = horizon - previous_time;
		const auto p_no_event = exp(- lambda_ * delta_time);
		// P(brak zdarzeń do czasu t1|czas t0) = exp( - (t1 - t0) * lambda )
		const auto u = rng.draw_uniform();
		if (u > p_no_event) {
			// Jest przynajmniej jedno zdarzenie!
			// Czas 1-szego zdarzenia t obliczamy z równania
			// u = exp( - (t - t0) * lambda )
			// - ln(u) = (t - t0) * lambda
			// t = t0 - ln(u) / lambda
			assert(u > 0);
			return previous_time - log(u) / lambda_;
		} else {
			return horizon;
		}
	}

	void from_json(const json& j, std::unique_ptr<SimplePoissonProcess>& process) {
		validate_keys(j, "SimplePoissonProcess", { }, { "lambda" , "inverse_lambda" });
		try {
			const bool has_lambda = j.count("lambda");
			const bool has_inverse_lambda = j.count("inverse_lambda");
			if (has_lambda && has_inverse_lambda) {
				throw DeserialisationError("SimplePoissonProcess", j.dump(), boost::locale::translate("\"lambda\" and \"inverse_lambda\" are mutually exclusive"));
			}
			double lambda;
			if (has_lambda) {
				lambda = j["lambda"];
			} else if (has_inverse_lambda) {
				lambda = 1. / get_seconds_from_json(j["inverse_lambda"]);
			} else {
				throw DeserialisationError("SimplePoissonProcess", j.dump(), boost::locale::translate("Either \"lambda\" or \"inverse_lambda\" must be provided"));
			}
			process = std::make_unique<SimplePoissonProcess>(lambda);
		} catch (const std::exception& e) {
			throw DeserialisationError("SimplePoissonProcess", j.dump(), e);
		}
	}
}
