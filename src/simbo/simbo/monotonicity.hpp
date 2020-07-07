#pragma once
#include "enums.hpp"
#include "exceptions.hpp"

namespace simbo {
	/// Status monotoniczności ciągu wartości.
	enum class Monotonicity {
		/// Stałe (x[i] == const).
		CONSTANT,

		/// Niemalejące (x[i] >= x[i-1]).
		NOT_DECREASING,

		/// Nierosnące (x[i] <= x[i-1]).
		NOT_INCREASING,

		/// Ściśle rosnące (x[i] > x[i-1]).
		STRICTLY_INCREASING,

		/// Ściśle malejące (x[i] < x[i-1]).
		STRICTLY_DECREASING,

		/// Brak monotoniczności.
		NONE
	};

	template <> struct EnumNames<Monotonicity> {
		static constexpr std::array<const char*, 6> NAMES = {
			"CONSTANT",
			"NOT_DECREASING",
			"NOT_INCREASING",
			"STRICTLY_INCREASING",
			"STRICTLY_DECREASING",
			"NONE"
		};
	};

	/** \brief Sprawdź czy wartości są monotoniczne i w jaki sposób.
	\param x Niepusty wektor.
	\return Stała najlepiej opisująca status monotoniczności.
	\throw std::invalid_argument Jeżeli x.empty().
	*/
	template <class Real> Monotonicity check_monotonicity(const std::vector<Real>& x) {
		if (x.empty()) {
			throw std::invalid_argument(boost::locale::translate("Empty X vector for monotonicity check"));
		}
		if (x.size() == 1) {
			return Monotonicity::CONSTANT;
		}
		bool not_increasing = true;
		bool not_decreasing = true;
		bool strictly_increasing = true;
		bool strictly_decreasing = true;
		auto prev_x = x.front();
		for (auto it = x.begin() + 1; it != x.end(); ++it) {
			const auto next_x = *it;
			if (next_x > prev_x) {
				strictly_decreasing = false;
				not_increasing = false;
			} else if (next_x < prev_x) {
				strictly_increasing = false;
				not_decreasing = false;
			} else {
				strictly_decreasing = false;
				strictly_increasing = false;
			}
			prev_x = next_x;
		}
		if (not_decreasing && not_increasing) {
			return Monotonicity::CONSTANT;
		} else if (strictly_decreasing) {
			return Monotonicity::STRICTLY_DECREASING;
		} else if (strictly_increasing) {
			return Monotonicity::STRICTLY_INCREASING;
		} else if (not_decreasing) {
			return Monotonicity::NOT_DECREASING;
		} else if (not_increasing) {
			return Monotonicity::NOT_INCREASING;
		} else {
			return Monotonicity::NONE;
		}
	}
}
