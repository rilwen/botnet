#pragma once
#include "interpolator1d.hpp"
#include "monotonicity.hpp"

namespace simbo {
	/**
	\brief Interpolator opisujący funkcję monotoniczną f(x).

	Funkcja monotoniczna charakteryzuje się tym, że dla dowolnych x1 <= x2 zachodzi zawsze albo f(x1) <= f(x2), albo f(x1) >= f(x2).
	*/
	template <class RealX, class RealY> class MonotonicInterpolator1D : public virtual Interpolator1D<RealX, RealY> {
	public:
		/**
		\brief Zwróć stan monotoniczności.
		\returns Wartość różna od Monotonicity::NONE.
		*/
		virtual Monotonicity get_monotonicity() const = 0;
	};
}
