#pragma once
#include "simbo/cycle.hpp"
#include "simbo/rng.hpp"

namespace simbo {
	/// \brief Udawany RNG.
	class MockRNG : public RNG {
	public:
		/// Konstruktor.
		MockRNG();

		double draw_uniform() override;

		double draw_gaussian() override;

		/// Dodaj wartość jednorodną do wylosowania.
		void add_uniform(double u);

		/// Zacznij wybierać wszystkie dodane wcześniej wartości od początku.
		void rewind();

		/// Zacznij wybierać wszystkie dodane wcześniej wartości od początku i zresetuj licznik pobrań.
		void reset();

		/// Usuń wszystkie dodane wartości i zresetuj licznik pobrań.
		void clear();

		/// Pobierz próbki z innego generatora.
		void add_from(RNG& other, int n);

		/// Zwróć liczbę podanych wartości.
		int get_number_draws() const {
			return number_draws_;
		}

		void discard(unsigned long long z) override;

		int_type draw_uniform_integer() override;
	private:
		/// Przechowuje losowe wartości o rozkładzie U(0, 1).
		Cycle<double> values_;

		/// Ile było pobranych wartości.
		int number_draws_;
	};
}
