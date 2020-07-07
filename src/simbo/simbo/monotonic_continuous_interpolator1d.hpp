#pragma once
#include "monotonic_interpolator1d.hpp"

namespace simbo {
	/**
	\brief Interpolator opisujący funkcję monotoniczną ciągłą f(x).

	Funkcja monotoniczna charakteryzuje się tym, że dla dowolnych x1 <= x2 zachodzi zawsze albo f(x1) <= f(x2), albo f(x1) >= f(x2).
	Funkcja ciągła charakteryzuje się tym, że lim_{x -> x0} f(x) = f(x0) dla wszystkich x z dziedziny funkcji.
	*/
	template <class RealX, class RealY> class MonotonicContinuousInterpolator1D : public virtual MonotonicInterpolator1D<RealX, RealY> {
	public:
		/**
		\brief Zwróć wartość X odpowiadającą wartości Y.

		Jeżeli f(x) == y dla x z przedzialu [x1, x2], zwróć x1.

		\throws std::domain_error Jeżeli w przedziale na którym interpolator jest określony nie ma takiego x, że f(x) == y.

		\returns Wartość z zakresu [#get_lower_bound(), #get_upper_bound()].
		*/
		virtual RealX invert_left(RealY y) const = 0;

		/**
		\brief Zwróć wartość X odpowiadającą wartości Y.

		Jeżeli f(x) == y dla x z przedzialu [x1, x2], zwróć x2.

		\throws std::domain_error Jeżeli w przedziale na którym interpolator jest określony nie ma takiego x, że f(x) == y.

		\returns Wartość z zakresu [#get_lower_bound(), #get_upper_bound()].
		*/
		virtual RealX invert_right(RealY y) const = 0;
	};
}