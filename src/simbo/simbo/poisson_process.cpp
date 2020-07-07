#include <boost/locale.hpp>
#include "json.hpp"
#include "monotonic_linear_interpolator1d.hpp"
#include "poisson_process.hpp"
#include "rng.hpp"
#include "schedule.hpp"

namespace simbo {
	PoissonProcess::PoissonProcess(interpolator1d_ptr&& int_lambda) {
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

	seconds_t PoissonProcess::get_next_jump(RNG& rng, const int previous_count, const seconds_t previous_time, const seconds_t horizon) const {
		check_times(previous_time, horizon);
		assert(previous_count >= get_min_state()); // Ponieważ get_min_state() == INT_MIN
		const auto int_lambda_final = (*int_lambda_)(horizon);
		const auto int_lambda_0 = (*int_lambda_)(previous_time);
		const auto p_no_event = exp(int_lambda_0 - int_lambda_final);
		// P(brak zdarzeń do czasu t1|czas t0) = exp( - int_t0^t1 lambda(s) ds )
		const auto u = rng.draw_uniform();
		if (u > p_no_event) {
			// Jest przynajmniej jedno zdarzenie!
			// Czas 1-szego zdarzenia t obliczamy z równania
			// u = exp( - int_t0^t lambda(s) ds )
			// - ln(u) = int_t0^t lambda(s) ds
			// int_0^t lambda(s) ds = int_0^t0 lambda(s) ds - ln(u)			
			assert(u > 0);
			return int_lambda_->invert_right((int_lambda_0 - log(u)));
		} else {
			return horizon;
		}
	}

	static void validate_lambdas_and_offsets(const std::string& category, const std::vector<double>& lambdas, const std::vector<TimeDuration>& offsets) {
		if (lambdas.size() != offsets.size() + 1) {
			throw std::invalid_argument((boost::locale::format("{1} offset vector size {2} incompatible with the intensity vector size {3}") % category % offsets.size() % lambdas.size()).str());
		}
		for (double lambda : lambdas) {
			if (!(lambda >= 0)) {
				throw std::domain_error((boost::locale::format("{1} intensity value {2} should be non-negative") % category % lambda).str());
			}
		}
		TimeDuration prev_offset;
		for (const TimeDuration& offset : offsets) {
			if (!(offset > prev_offset)) {
				throw std::invalid_argument((boost::locale::format("{1} offset {2} should be larger than the previous one {3}") % category % offset % prev_offset).str());
			}
			if (!(to_seconds(offset) < 24 * 3600)) {
				throw std::invalid_argument((boost::locale::format("{1} offset {2} should be less than 24h") % category % offset).str());
			}
			prev_offset = offset;
		}
	}

	void from_json_seasonal_weekly(const json& j, const Schedule& schedule, std::unique_ptr<PoissonProcess>& process) {
		validate_keys(j, "PoissonProcess", { "workday_lambdas", "weekend_lambdas" }, { "workday_offsets", "weekend_offsets" });
		try {
			// Zmienne _offsets powinny zawierać przesunięcia czasowe (od początku dnia) do momentów przejść pomiędzy
			// kolejnymi wartościami intensywności lambda.
			const std::vector<double> workday_lambdas = j["workday_lambdas"];
			std::vector<TimeDuration> workday_offsets;
			if (j.count("workday_offsets")) {
				workday_offsets = j["workday_offsets"].get<std::vector<TimeDuration>>();
			}
			const std::vector<double> weekend_lambdas = j["weekend_lambdas"];
			std::vector<TimeDuration> weekend_offsets;
			if (j.count("weekend_offsets")) {
				weekend_offsets = j["weekend_offsets"].get<std::vector<TimeDuration>>();
			}
			validate_lambdas_and_offsets("workday", workday_lambdas, workday_offsets);
			validate_lambdas_and_offsets("weekend", weekend_lambdas, weekend_offsets);
			// Dodaj punkty startowe.
			workday_offsets.insert(workday_offsets.begin(), TimeDuration());
			weekend_offsets.insert(weekend_offsets.begin(), TimeDuration());
			const DateTime& ref_datetime = schedule.get_point(0);
			const Date start_date = ref_datetime.date();
			const int nbr_days = (schedule.get_point(schedule.size() - 1).date() - start_date).days() + 1;
			Schedule process_schedule(start_date, workday_offsets, weekend_offsets, nbr_days, true);
			Date date = start_date;
			const auto one_day = boost::gregorian::days(1);
			std::vector<double> process_lambdas;
			for (int i = 0; i < nbr_days; ++i) {
				const std::vector<double>& lambdas = is_weekend(date) ? weekend_lambdas : workday_lambdas;
				process_lambdas.insert(process_lambdas.end(), lambdas.begin(), lambdas.end());
				date += one_day;
			}
			process_lambdas.shrink_to_fit();
			PoissonProcess::interpolator1d_ptr int_lambda_interpolator(new MonotonicLinearInterpolator1D<seconds_t, double>(
				process_schedule.get_seconds(),
				std::move(process_lambdas), 0));
			process = std::make_unique<PoissonProcess>(std::move(int_lambda_interpolator));
		} catch (std::exception& e) {
			throw DeserialisationError("PoissonProcess", j.dump(), e);
		}
	}
}
