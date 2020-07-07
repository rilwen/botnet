#pragma once
#include <random>
#include "rng.hpp"

namespace simbo {
	/// Domyślna implementacja interfejsu RNG.
	class RNGImpl : public RNG {
	public:
		/**
		Konstruktor.

		\param seed Ziarno generatora.
		*/
		RNGImpl(int_type seed);

		double draw_uniform() override;

		double draw_gaussian() override;

		int_type draw_uniform_integer() override;

		void discard(unsigned long long z) override;
	private:
		/// Silnik generatora liczb losowych.
		std::mt19937_64 engine_;

		/// Dystrybucja U(0, 1).
		std::uniform_real_distribution<double> uniform01_;

		/// Dystrybucja N(0, 1).
		std::normal_distribution<double> normal01_;
	};
}
