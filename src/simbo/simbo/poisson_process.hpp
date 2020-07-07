#pragma once
#include <memory>
#include "monotonic_continuous_interpolator1d.hpp"
#include "jump_markov_process.hpp"

namespace simbo {
	
	class Schedule;

	/**
	\brief Process Poissona o zależnej od czasu intensywności.

	Stan procesu to liczba całkowita, zwiększana o 1 kiedy proces zarejestruje wydarzenie.
	*/
	class PoissonProcess : public JumpMarkovProcess {
	public:
		/// Deklaracje typów.
		typedef std::unique_ptr<const MonotonicContinuousInterpolator1D<seconds_t, double>> interpolator1d_ptr;

		/**
		\brief Konstruktor.
		\param int_lambda Interpolator intensywności scałkowanej po czasie: \f$\text{int_lambda}(t) = \int_{t_0}^t \lambda(s) ds \f$
		\throws std::invalid_argument Jeżeli int_lambda == nullptr. Jeżeli int_lambda->get_lower_bound() == int_lambda->get_upper_bound(). Jeżeli int_lambda->get_monotonicity() != Monotonicity::NOT_DECREASING.
		*/
		PoissonProcess(interpolator1d_ptr&& int_lambda);

		seconds_t get_first_time() const override {
			return int_lambda_->get_lower_bound();
		}

		seconds_t get_last_time() const override {
			return int_lambda_->get_upper_bound();
		}
		
		seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const override;

		int get_min_state() const override {
			return std::numeric_limits<int>::min();
		}
	private:
		/// Intensywność.
		interpolator1d_ptr int_lambda_;
	};

	/** \brief Odczytaj PoissonProcess z formatu JSON.
	Funkcja konstruuje proces który zachowuje się w jeden sposób w dni robocze, a w inny w weekend.
	Wymagane pola: "workday_lambdas", "weekend_lambdas". Opcjonalne pola: "workday_offsets", "weekend_offsets".
	\param j Dane JSON.
	\param schedule Harmonogram symulacji.
	\param process Referencja do wskaźnika do tworzonego procesu. Po udanym wywołaniu funkcji process != nullptr.
	*/
	void from_json_seasonal_weekly(const json& j, const Schedule& schedule, std::unique_ptr<PoissonProcess>& process);
}
