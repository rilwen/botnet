#pragma once
#include "monotonic_interpolator1d.hpp"
#include "jump_markov_process.hpp"

namespace simbo {

	/**
	\brief Process Poissona o stałej intensywności mnożonej przez dotychczasową liczbę wydarzeń.

	Stan procesu to nieujemna liczba całkowita (liczba wydarzeń które zaszły do tej pory), zwiększana o 1 kiedy proces zarejestruje wydarzenie.
	*/
	class SimpleAutoscaledPoissonProcess : public JumpMarkovProcess {
	public:
		/**
		\brief Konstruktor.
		\param lambda Stała intensywność.
		\throws std::domain_error Jeżeli lambda < 0 
		*/
		SimpleAutoscaledPoissonProcess(double lambda);

		seconds_t get_first_time() const override {
			return -infinite_time();
		}

		seconds_t get_last_time() const override {
			return infinite_time();
		}

		seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const override;

		int get_min_state() const override {
			return 0;
		}

		/// Zwróć wartość intensywności.
		/// \return Nieujemna liczba.
		double get_lambda() const {
			return lambda_;
		}
	private:
		/// Intensywność.
		double lambda_;
	};
}
