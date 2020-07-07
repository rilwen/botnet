#pragma once
#include <vector>
#include <boost/locale.hpp>
#include "interpolator1d_impl.hpp"
#include "json.hpp"
#include "markov_process.hpp"
#include "rng.hpp"
#include "schedule.hpp"

namespace simbo {

	/**
	\brief Pół-deterministyczny proces Markowa.

	Proces który zmienia swój stan w trakcie "okresów przejściowych". W okresie [t_{i,0}, t_{i,1})
	proces ma deterministyczną wartość k_i. W chwili \f$ t_{i,s} \in [ t_{i,1}, t_{i+1,0} ) \f$ proces zmienia wartość na k_{i+1}. Wartość
	t_{i,s} jest losowa z rozkładem jednorodnym na przedziale [t_{i,1}, t_{i+1,0}). Dokładniej, jeżeli w chwili t_{i,1} <= t < t_{i+1,0} proces ma nadal wartość k_i, to prawdopodobieństwo że w przedziale czasu (t, t'), t' <= t_{i+1,0} zmieni wartość na
	k_{i+1} jest równe (t' - t) / (t_{i+1, 0} - t).

	\tparam State Typ reprezentujący stan procesu.
	*/
	template <class State> class SemiPredeterminedProcess : public MarkovProcess<State> {
	public:
		/**
		\brief Konstruktor
		\param t0 Wektor czasów t0 (początków okresów deterministycznych).
		\param t1 Wektor czasów t1 (końców okresów deterministycznych).
		\param values Wartości procesu.
		\throws std::invalid_argument Jeżeli t0.size() != t1.size() != values.size() < 1, albo jeżeli t0.front() >= t1.back(), albo jeżeli t0[i] > t1[i] dla któregokolwiek i, albo t1[i - 1] > t0[i] dla i > 0.
		*/
		SemiPredeterminedProcess(std::vector<seconds_t>&& t0, std::vector<seconds_t>&& t1, std::vector<State>&& values) {
			const auto n = values.size();
			if (!n) {
				throw std::invalid_argument(boost::locale::translate("No values provided"));
			}
			if (t0.size() != n || t1.size() != n) {
				throw std::invalid_argument(boost::locale::translate("Wrong size(s) of time vector(s)"));
			}
			if (!(t0.front() < t1.back())) {
				throw std::invalid_argument((boost::locale::format("Wrong time range {1} to {2}") % t0.front() % t1.back()).str());
			}
			for (auto i = static_cast<int>(n - 1); i >= 0; --i) {
				if (!(t0[i] <= t1[i])) {
					throw std::invalid_argument((boost::locale::format("Reversed deterministic range boundaries ({1} to {2}) at position {3}") % t0[i] % t1[i] % i).str());
				}
				if (i > 0) {
					if (!(t1[i - 1] <= t0[i])) {
						throw std::invalid_argument((boost::locale::format("Overlapping deterministic range boundaries between position {1} and {2}") % (i - 1) % i).str());
					}
					// Wynika z poprzednich warunków.
					assert(t0[i - 1] <= t0[i]);
					assert(t1[i - 1] <= t1[i]);
				}
			}
			t0_ = std::move(t0);
			t1_ = std::move(t1);
			values_ = std::move(values);
		}

		seconds_t get_first_time() const override {
			return t0_.front();
		}

		seconds_t get_last_time() const override {
			return t1_.back();
		}

		State get_next_state(RNG& rng, const State& previous_state, const seconds_t previous_time, const seconds_t next_time) const override {
			check_times(previous_time, next_time);
			int i0;
			int i1;
			get_time_indices(previous_time, i0, i1);
			// Sprawdź czy previous_state jest zgodny z historią.
			if (i0 >= i1) {
				// t0[i0] <= previous_time <= t1[i1] <= t1[i0]
				// W zakresie deterministycznym.
				if (values_[i0] != previous_state) {
					throw std::domain_error(boost::locale::translate("Previous state incompatible with history"));
				}
			} else {
				assert(i1 == i0 + 1);
				if (previous_state != values_[i0] && previous_state != values_[i1]) {
					throw std::domain_error(boost::locale::translate("Previous state incompatible with history"));
				}
			}
			int j0;
			int j1;
			get_time_indices(next_time, j0, j1);
			assert(j0 >= i0);
			assert(j1 >= i1);
			if (j0 >= j1) {
				// t0[j0] <= previous_time <= t1[j1] <= t1[j0]
				// W zakresie deterministycznym.
				return values_[j0];
			} else {
				assert(j1 == j0 + 1);
				if (previous_state == values_[j1]) {
					// Przejście już zaszło.
					return previous_state;
				} else {
					const double from = std::max(t1_[j0], previous_time);
					const double to = t0_[j1];
					assert(next_time >= from);
					assert(next_time <= to);
					if (to > from) {
						// Czas przejścia ma rozkład jednorodny.
						const double p = (next_time - from) / (t0_[j1] - from);
						assert(p >= 0);
						assert(p <= 1);
						const double u = rng.draw_uniform();
						return (u < p) ? values_[j1] : values_[j0];
					} else {
						assert(next_time == from);
						// Natychmiastowe przejście do nowej wartości.
						return values_[j1];
					}
				}
			}
		}

		/// W okresie deterministycznym zwraca wartość z historii, w okresie przejściowym losuje nową wartość zgodnie z rozkładem jednorodnym.
		State instant_switch(RNG& rng, const State& other_process_state, const seconds_t time) const override {
			check_time(time);
			int i0;
			int i1;
			get_time_indices(time, i0, i1);
			if (i0 >= i1) {
				// t0[i0] <= time <= t1[i1] <= t1[i0]
				// W zakresie deterministycznym.
				return values_[i0];
			} else {
				assert(i1 == i0 + 1);
				const double from = t1_[i0];
				const double to = t0_[i1];
				assert(time >= from);
				assert(time <= to);
				if (to > from) {
					// Czas przejścia ma rozkład jednorodny.
					const double p = (time - from) / (t0_[i1] - from);
					assert(p >= 0);
					assert(p <= 1);
					const double u = rng.draw_uniform();
					return (u < p) ? values_[i1] : values_[i0];
				} else {
					assert(time == from);
					// Natychmiastowe przejście do nowej wartości.
					return values_[i1];
				}
			}
		}

		/** \brief Stwórz proces który ma określone okresy i wartości deterministyczne w każdy dzień roboczy, inne zaś w każdy weekend.
		Weekend oznacza tutaj sobotę i niedzielę. Krańce okresów mają być podane jako przesunięcia czasowe od początku dnia i nie mogą przekraczać 24 godzin.
		\param deterministic_periods_workdays Wektor okresów deterministycznych w dni robocze.
		\param deterministic_values_workdays Wektor wartości deterministycznych w dni robocze.
		\param deterministic_periods_weekend Wektor okresów deterministycznych w weekend.
		\param deterministic_values_weekend Wektor wartości deterministycznych w weekend.
		\param reference_date_time Data-czas od którego liczony jest czas w sekundach dla każdego początku/końca okresu deterministycznego.
		\param start_date Początkowy dzień na którym określony jest proces.
		\param end_date Końcowy dzień na którym określony jest proces.
		\throws std::invalid_argument Jeżeli długości wektorów okresów i odpowiadających im wartości są różne. Jeżeli okresy się przekrywają albo ich początki następują po ich końcach. Jeżeli end_date < start_date.
		*/
		static std::unique_ptr<SemiPredeterminedProcess<State>> make_seasonal_weekly(
			const std::vector<std::pair<TimeDuration, TimeDuration>>& deterministic_periods_workdays,
			const std::vector<State>& deterministic_values_workdays,
			const std::vector<std::pair<TimeDuration, TimeDuration>>& deterministic_periods_weekend,
			const std::vector<State>& deterministic_values_weekend,
			const DateTime& reference_date_time,
			const Date& start_date,
			const Date& end_date
		) {
			if (deterministic_periods_workdays.size() != deterministic_values_workdays.size()) {
				throw std::invalid_argument(boost::locale::translate("Period and values vectors for workdays have different lengths"));
			}
			if (deterministic_periods_weekend.size() != deterministic_values_weekend.size()) {
				throw std::invalid_argument(boost::locale::translate("Period and values vectors for weekend have different lengths"));
			}
			validate_deterministic_periods(deterministic_periods_workdays);
			validate_deterministic_periods(deterministic_periods_weekend);
			const auto one_day = boost::gregorian::days(1);
			std::vector<seconds_t> t0;
			std::vector<seconds_t> t1;
			std::vector<State> values;
			for (Date date = start_date; date <= end_date; date += one_day) {
				const bool is_this_weekend = is_weekend(date);
				const std::vector<std::pair<TimeDuration, TimeDuration>>& deterministic_periods = is_this_weekend ? deterministic_periods_weekend : deterministic_periods_workdays;
				const std::vector<State>& deterministic_values = is_this_weekend ? deterministic_values_weekend : deterministic_values_workdays;
				const size_t n = deterministic_periods.size();
				for (size_t i = 0; i < n; ++i) {
					values.push_back(deterministic_values[i]);
					const auto& period = deterministic_periods[i];
					DateTime period_start(date, period.first);
					DateTime period_end(date, period.second);
					t0.push_back(to_seconds(period_start - reference_date_time));
					t1.push_back(to_seconds(period_end - reference_date_time));
				}
			}
			t0.shrink_to_fit();
			t1.shrink_to_fit();
			values.shrink_to_fit();
			return std::make_unique<SemiPredeterminedProcess<State>>(std::move(t0), std::move(t1), std::move(values));
		}

		/// Zwróć referencję do wektora dolnych krańców okresów deterministycznych.
		const std::vector<seconds_t>& get_lower_bounds() const {
			return t0_;
		}

		/// Zwróć referencję do wektora górnych krańców okresów deterministycznych.
		const std::vector<seconds_t>& get_upper_bounds() const {
			return t1_;
		}

		/// Zwróć referencję do wektora deterministycznych wartości.
		const std::vector<State>& get_values() const {
			return values_;
		}
	private:
		/// Dolne zakresy okresów deterministycznych.
		std::vector<seconds_t> t0_;

		/// Górne zakresy okresów deterministycznych.
		std::vector<seconds_t> t1_;

		/// Wartości procesu.
		std::vector<State> values_;

		void get_time_indices(const seconds_t time, int& i0, int& i1) const {
			const auto it0 = find_left_node(t0_.begin(), t0_.end(), time);
			i0 = static_cast<int>(std::distance(t0_.begin(), it0));
			const auto it1 = find_right_node(t1_.begin(), t1_.end(), time);
			i1 = static_cast<int>(std::distance(t1_.begin(), it1));
			assert(std::abs(i1 - i0) <= 1);
		}

		static void validate_deterministic_periods(const std::vector<std::pair<TimeDuration, TimeDuration>>& deterministic_periods) {
			for (size_t i = 0; i < deterministic_periods.size(); ++i) {
				const auto& period = deterministic_periods[i];
				if (!(period.first <= period.second)) {
					throw std::invalid_argument((boost::locale::format("Period bounds {1}, {2} out of order") % period.first % period.second).str());
				}
				if (!(to_seconds(period.first) >= 0)) {
					throw std::domain_error((boost::locale::format("Period bound {1} is negative") % period.first).str());
				}
				if (!(to_seconds(period.second) < 24 * 3600)) {
					throw std::domain_error((boost::locale::format("Period bound {1} should be less then 24 hours") % period.second).str());
				}
			}
		}
	};

	/** \brief Odczytaj SemiPredeterminedProcess z formatu JSON.
	Funkcja konstruuje proces który zachowuje się w jeden sposób w dni robocze, a w inny w weekend.
	\tparam State Typ stanu procesu.
	\param j Dane JSON.
	\param schedule Harmonogram symulacji.
	\param process Referencja do wskaźnika do tworzonego procesu. Po udanym wywołaniu funkcji process != nullptr.
	*/
	template <class State> void from_json_seasonal_weekly(const json& j, const Schedule& schedule, std::unique_ptr<SemiPredeterminedProcess<State>>& process) {
		const auto one_day = boost::gregorian::days(1);
		const DateTime reference_date_time = schedule.get_point(0);
		const Date start_date = reference_date_time.date() - one_day;
		const Date end_date = schedule.get_point(schedule.size() - 1).date() + one_day;
		validate_keys(j, "SemiPredeterminedProcess", { "deterministic_periods_workdays", "deterministic_values_workdays", "deterministic_periods_weekend", "deterministic_values_weekend" }, {});
		try {
			const std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_period_workdays = j["deterministic_periods_workdays"];
			const std::vector<State> deterministic_values_workdays = j["deterministic_values_workdays"];
			const std::vector<std::pair<TimeDuration, TimeDuration>> deterministic_period_weekend = j["deterministic_periods_weekend"];
			const std::vector<State> deterministic_values_weekend = j["deterministic_values_weekend"];
			process = SemiPredeterminedProcess<State>::make_seasonal_weekly(deterministic_period_workdays, deterministic_values_workdays, deterministic_period_weekend, deterministic_values_weekend, reference_date_time, start_date, end_date);
		} catch (const DeserialisationError&) {
			throw;
		} catch (const std::exception& e) {
			throw DeserialisationError("SemiPredeterminedProcess", j.dump(), e);
		}
	}
}
