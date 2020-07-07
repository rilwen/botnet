#pragma once
#include "monotonic_continuous_interpolator1d.hpp"
#include "jump_markov_process.hpp"

namespace simbo {

	class RNG;

	/**
	\brief Process Poissona o zależnej od czasu intensywności mnożonej przez dotychczasową liczbę wydarzeń.

	Stan procesu to nieujemna liczba całkowita (liczba wydarzeń które zaszły do tej pory), zwiększana o 1 kiedy proces zarejestruje wydarzenie.
	*/
	class AutoscaledPoissonProcess : public JumpMarkovProcess {
	public:
		/// Deklaracje typów.
		typedef std::unique_ptr<const MonotonicContinuousInterpolator1D<seconds_t, double>> interpolator1d_ptr;

		/**
		\brief Konstruktor.
		\param int_lambda Interpolator intensywności scałkowanej po czasie: \f$\text{int_lambda}(t) = \int_{t_0}^t \lambda(s) ds \f$
		\throws std::invalid_argument Jeżeli int_lambda == nullptr. Jeżeli int_lambda->get_lower_bound() == int_lambda->get_upper_bound(). Jeżeli int_lambda->get_monotonicity() != Monotonicity::NOT_DECREASING.
		*/
		AutoscaledPoissonProcess(interpolator1d_ptr&& int_lambda);

		seconds_t get_first_time() const override {
			return int_lambda_->get_lower_bound();
		}

		seconds_t get_last_time() const override {
			return int_lambda_->get_upper_bound();
		}

		seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const override;

		int get_min_state() const override {
			return 0;
		}
	private:
		/// Intensywność.
		interpolator1d_ptr int_lambda_;
	};
}
