#pragma once
#include <boost/locale.hpp>
#include "linear_interpolator1d.hpp"
#include "monotonic_continuous_interpolator1d.hpp"
#include "utils.hpp"

namespace simbo {
	/// Monotoniczny interpolator liniowy.
	template <class RealX, class RealY> class MonotonicLinearInterpolator1D: public LinearInterpolator1D<RealX, RealY>, public  MonotonicContinuousInterpolator1D<RealX, RealY> {
	public:
		/**
		\brief Konstruktor.
		\param xs Zobacz Interpolator1DImpl#Interpolator1DImpl.
		\param ys Zobacz Interpolator1DImpl#Interpolator1DImpl.
		\throws std::invalid_argument Jeżeli węzły Y nie są monotoniczne. Zobacz również Interpolator1DImpl#Interpolator1DImpl.
		*/
		MonotonicLinearInterpolator1D(std::vector<RealX>&& xs, const std::vector<RealY>& ys)
			: LinearInterpolator1D<RealX, RealY>(std::move(xs), make_copy(ys)), monotonicity_(check_monotonicity(ys)) {
			if (monotonicity_ == Monotonicity::NONE) {
				throw std::invalid_argument(boost::locale::translate("Y vector is not monotonic"));
			}
		}

		/**
		\brief Konstruktor całkujący. Zobacz LinearInterpolator1D#LinearInterpolator1D(std::vector<RealX>&&, const std::vector<RealY>&, RealY)
		\param xs Węzły X
		\param slopes Pochodne pomiędzy węzłami.
		\param initial_value Początkowa wartość całki.
		\throws std::invalid_argument Jeżeli obliczone węzły Y nie są monotoniczne.
		*/
		MonotonicLinearInterpolator1D(std::vector<RealX>&& xs, const std::vector<RealY>& slopes, RealY initial_value)
			: LinearInterpolator1D<RealX, RealY>(std::move(xs), slopes, initial_value),
			monotonicity_(check_monotonicity_from_integrand(slopes)) {
			if (monotonicity_ == Monotonicity::NONE) {
				throw std::invalid_argument(boost::locale::translate("Y vector is not monotonic"));
			}
		}

		RealX invert_left(const RealY y) const override {
			return invert(y, true);
		}

		RealX invert_right(const RealY y) const override {
			return invert(y, false);
		}

		Monotonicity get_monotonicity() const override {
			return monotonicity_;
		}
	private:
		RealX invert(const RealY y, const bool left) const {
			const auto& xs = get_xs();
			const auto& ys = get_ys();
			if (xs.size() == 1) {
				if (y == ys.front()) {
					return xs.front();
				} else {
					throw std::domain_error(boost::locale::translate("Y value outside interpolator range"));
				}
			} else {
				if (monotonicity_ == Monotonicity::NOT_DECREASING) {
					return invert_impl(ys.begin(), ys.end(), xs.begin(), xs.end(), y, left);
				} else {
					assert(monotonicity_ == Monotonicity::NOT_INCREASING);
					return invert_impl(ys.rbegin(), ys.rend(), xs.rbegin(), xs.rend(), y, !left);
				}
			}
		}

		template <class IterX, class IterY> static RealX invert_impl(const IterY begin_y, const IterY end_y, const IterX begin_x, const IterX end_x, const RealY y, const bool left) {
			auto lb = std::lower_bound(begin_y, end_y, y);
			if (lb == end_y) {
				throw std::domain_error(boost::locale::translate("Y value outside interpolator range"));
			}
			if (!left) {
				// Znajdź ostatni węzeł dla którego ys[i] == y.
				while (true) {
					auto next = lb + 1;
					if (next == end_y || *next > y) {
						break;
					} else {
						lb = next;
					}
				}
			}
			assert(lb != end_y);
			assert(*lb >= y);
			const auto y1 = *lb;
			const auto x_it = begin_x + std::distance(begin_y, lb);
			const auto x1 = *x_it;
			// Oblicz x.
			if (y1 == y) {
				return x1;
			} else {
				if (lb == begin_y) {
					throw std::domain_error(boost::locale::translate("Y value outside interpolator range"));
				} else {
					const auto y0 = *(lb - 1);
					assert(y0 < y);
					assert(y1 > y);
					const auto x0 = *(x_it - 1);
					const auto dy = y1 - y0;
					return x0 * ((y1 - y) / dy) + x1 * ((y - y0) / dy);
				}
			}
		}

		/// Sprawdź czy wartości funkcji podcałkowej prowadzą do monotonicznej funkcji po scałkowaniu.
		static Monotonicity check_monotonicity_from_integrand(const std::vector<RealY>& slopes) {
			if (slopes.empty()) {
				return Monotonicity::NOT_DECREASING;
			}
			bool all_nonnegative = true;
			bool all_nonpositive = true;
			for (auto it = slopes.begin(); it != slopes.end(); ++it) {
				const auto slope = *it;
				all_nonnegative &= slope >= RealY(0);
				all_nonpositive &= slope <= RealY(0);
			}
			if (all_nonnegative) {
				return Monotonicity::NOT_DECREASING;
			} else if (all_nonpositive) {
				return Monotonicity::NOT_INCREASING;
			} else {
				return Monotonicity::NONE;
			}
		}

		Monotonicity monotonicity_;
	};
}
