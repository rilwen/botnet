#pragma once
#include <cassert>
#include "exceptions.hpp"
#include "interpolator1d_impl.hpp"

namespace simbo {

	/// Interpolator kawałkami stały, ekstrapolujący do przodu: y(x) = xs[i] dla i = argmax xs[i] t. że xs[i] <= x.
	template <class RealX, class RealY> class PiecewiseConstantInterpolator1D: public Interpolator1DImpl<RealX, RealY> {
	public:
		/** \brief Konstruktor.
		\param xs Wektor węzłów X.
		\param ys Wektor węzłów Y. ys.size() + 1 == xs.size() albo ys.size() == xs.size().
		\throw Jeżeli rozmiary xs i ys są niepoprawne.
		\see Interpolator1DImpl
		*/
		PiecewiseConstantInterpolator1D(std::vector<RealX>&& xs, std::vector<RealY>&& ys)
			: Interpolator1DImpl<RealX, RealY>(std::move(xs), std::move(extend_ys(xs, ys))) {
		}

		/** \brief Konstruktor punktowy.

		Konstruktor buduje interpolator o wartości y na dla punktu x.

		\param x Argument.
		\param y Zwracana wartość.
		*/
		PiecewiseConstantInterpolator1D(const RealX& x, const RealY& y)
			: PiecewiseConstantInterpolator1D<RealX, RealY>(std::vector<RealX>({ x }), std::vector<RealY>({ y })) {}

		/** \brief Konstruktor jednoodcinkowy.

		Konstruktor buduje interpolator zwracający wartość y na odcinku [min_x, max_x].

		\param min_x Początek odcinka.
		\param max_x Koniec odcinka.
		\param y Zwracana wartość.
		*/
		PiecewiseConstantInterpolator1D(const RealX& min_x, const RealX& max_x, const RealY& y)
			: PiecewiseConstantInterpolator1D<RealX, RealY>(std::vector<RealX>({ min_x, max_x }), std::vector<RealY>({ y, y })) {}

		/// Konstruktor kopiujący.
		PiecewiseConstantInterpolator1D(const PiecewiseConstantInterpolator1D<RealX, RealY>& other)
			: Interpolator1DImpl<RealX, RealY>(other) {}

		/// Konstruktor domyślny.
		PiecewiseConstantInterpolator1D()
			: Interpolator1DImpl<RealX, RealY>(std::vector<RealX>(1), std::vector<RealY>(1)) {}

		PiecewiseConstantInterpolator1D<RealX, RealY>& operator=(PiecewiseConstantInterpolator1D<RealX, RealY>&& other) {
			Interpolator1DImpl<RealX, RealY>::operator=(std::move(other));
			return *this;
		}

		RealY evaluate(RealX x, int left_node_index) const override {
			assert(left_node_index >= 0);
			assert(left_node_index < size());
			assert(x >= get_xs()[left_node_index]);
			assert(left_node_index + 1 == size() || x <= get_xs()[left_node_index + 1]);
			return get_ys()[left_node_index];
		}		
	private:
		std::vector<RealY>& extend_ys(const std::vector<RealX>& xs, std::vector<RealY>& ys) {
			if (xs.empty()) {
				throw std::invalid_argument(boost::locale::translate("Xs vector is empty"));
			}
			if (ys.empty()) {
				throw std::invalid_argument(boost::locale::translate("Ys vector is empty"));
			}
			if (ys.size() + 1 == xs.size()) {
				// Dostosuj do wymagań klasy podstawowej.
				ys.push_back(ys.back());
			} else if (ys.size() != xs.size()) {
				throw std::invalid_argument((boost::locale::format("Ys vector size {1} incompatible with Xs vector size {2}") % ys.size() % xs.size()).str());
			}
			return ys;
		}
	};
}
