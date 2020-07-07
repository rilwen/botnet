#pragma once
#include <memory>
#include "interpolator1d.hpp"
#include "markov_process.hpp"

namespace simbo {

	class RNG;

	/// Ruch Browna o zadanej wolatywności i dryfcie.
	class BrownianMotion : public MarkovProcess<double> {
	public:
		/// Deklaracje typów.
		typedef std::unique_ptr<const Interpolator1D<seconds_t, double>> interpolator1d_ptr;

		/**
		\brief Konstruktor.
		\param int_volatility_squared Interpolator kwadratowej wolatywności scałkowanej po czasie: \f$ \text{int_volatility_squared}(t) = \int_{t_0}^t \sigma(s)^2 ds \f$.
		\param int_drift Interpolator dryftu scałkowanego po czasie: \f$ \text{int_drift}(t) = \int_{t_0}^t \mu(s) ds \f$.
		\throws std::invalid_argument Jeżeli int_volatility_squared == nullptr, int_drift == nullptr. Jeżeli zakresy na których interpolatory wolatywności i dryftu są określone nie przekrywają się na niezerowym odcinku.
		*/
		BrownianMotion(interpolator1d_ptr&& int_volatility_squared, interpolator1d_ptr&& int_drift);

		seconds_t get_first_time() const override {
			return first_time_;
		}

		seconds_t get_last_time() const override {
			return last_time_;
		}

		double get_next_state(RNG& rng, const double& previous_state, seconds_t previous_time, seconds_t next_time) const override;

		/// Zwraca wartość other_process_state.
		double instant_switch(RNG& rng, const double& other_process_state, seconds_t time) const override;
	private:
		/// Kwadratowa wolatywność (sigma_t^2) scałkowana po czasie.
		interpolator1d_ptr int_vol_sqr_;

		/// Dryft scałkowany po czasie.
		interpolator1d_ptr int_drift_;

		/// Początek zakresu czasowego.
		seconds_t first_time_;

		/// Koniec zakresu czasowego.
		seconds_t last_time_;
	};
}
