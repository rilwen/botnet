#pragma once
#include <boost/locale.hpp>
#include "monotonic_interpolator1d.hpp"
#include "piecewise_constant_interpolator1d.hpp"

namespace simbo {
	/// Monotoniczny interpolator kawałkami stały.
	template <class RealX, class RealY> class MonotonicPiecewiseConstantInterpolator1D : public PiecewiseConstantInterpolator1D<RealX, RealY>, public  MonotonicInterpolator1D<RealX, RealY> {
	public:
		/**
		\brief Konstruktor.
		\param xs Zobacz Interpolator1DImpl#Interpolator1DImpl.
		\param ys Zobacz Interpolator1DImpl#Interpolator1DImpl.
		\throws std::invalid_argument Jeżeli wartości Y nie są monotoniczne. Zobacz również Interpolator1DImpl#Interpolator1DImpl.
		*/
		MonotonicPiecewiseConstantInterpolator1D(std::vector<RealX>&& xs, const std::vector<RealY>& ys)
			: PiecewiseConstantInterpolator1D<RealX, RealY>(std::move(xs), make_copy(ys)), monotonicity_(check_monotonicity(ys)) {
			if (monotonicity_ == Monotonicity::NONE) {
				throw std::invalid_argument(boost::locale::translate("Y vector is not monotonic"));
			}			
		}

		/// Konstruktor konwertujący niekoniecznie monotoniczny interpolator kawałkami stały.
		/// \throw std::invalid_argument Jeżeli wartości Y nie są monotoniczne.
		MonotonicPiecewiseConstantInterpolator1D(const PiecewiseConstantInterpolator1D<RealX, RealY>& other)
			: MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>(make_copy(other.get_xs()), make_copy(other.get_ys())) {}

		/// Konstruktor kopiujący.
		MonotonicPiecewiseConstantInterpolator1D(const MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>& other)
			: PiecewiseConstantInterpolator1D<RealX, RealY>(other), monotonicity_(other.monotonicity_)			
		{}

		/// Konstruktor domyślny.
		MonotonicPiecewiseConstantInterpolator1D()
			: MonotonicPiecewiseConstantInterpolator1D(std::vector<RealX>(1), std::vector<RealY>(1)) {}

		/// Przenoszący operator przypisania.
		MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>& operator=(MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>&& other) {
			if (this != &other) {
				PiecewiseConstantInterpolator1D<RealX, RealY>::operator=(std::move(other));
				monotonicity_ = other.monotonicity_;
			}
			return *this;
		}

		Monotonicity get_monotonicity() const override {
			return monotonicity_;
		}
	private:
		Monotonicity monotonicity_;
	};
}