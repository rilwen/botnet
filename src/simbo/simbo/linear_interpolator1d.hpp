#pragma once
#include <cassert>
#include <memory>
#include <boost/locale.hpp>
#include "interpolator1d_impl.hpp"

namespace simbo {
	/// Interpolator liniowy.
	template <class RealX, class RealY> class LinearInterpolator1D: public Interpolator1DImpl<RealX, RealY> {
	public:
		using Interpolator1DImpl<RealX, RealY>::Interpolator1DImpl;

		/**
		\brief Konstruktor całkujący funkcję kawałkami stałą i tworzący funkcję ciągłą, kawałkami liniową.
		\param xs Węzły X funkcji.
		\param slopes Węzły Y funkcji podcałkowej.
		\param initial_value Początkowa wartość całki.
		\throws std::invalid_argument Jeżeli xs albo ys są puste albo slopes.size() + 1 < xs.size(). Zobacz również Interpolator1DImpl<RealX, RealY>#Interpolator1DImpl.
		*/
		LinearInterpolator1D(std::vector<RealX>&& xs, const std::vector<RealY>& slopes, RealY initial_value)
			: LinearInterpolator1D<RealX, RealY>(std::move(xs), integrate(xs, slopes, initial_value)) {
		}

		RealY evaluate(const RealX x, const int left_node_index) const override {
			assert(left_node_index >= 0);
			assert(left_node_index < size());
			const auto right_node_index = left_node_index + 1;
			if (right_node_index < size()) {
				// Ogólny przypadek.
				const auto left_x = get_xs()[left_node_index];
				const auto right_x = get_xs()[right_node_index];
				const auto width = right_x - left_x;
				const auto left_weight = (right_x - x) / width;
				const auto right_weight = (x - left_x) / width;
				return get_ys()[left_node_index] * left_weight + get_ys()[right_node_index] * right_weight;
			} else {
				assert(x == get_upper_bound());
				return get_ys().back();
			}
		}
	private:
		static std::vector<RealY> integrate(const std::vector<RealX>& xs, const std::vector<RealY>& slopes, const RealY initial_value) {
			const auto size = xs.size();
			if (!size) {
				throw std::invalid_argument(boost::locale::translate("X vector is empty"));
			}
			if (slopes.size() + 1 < size) {
				throw std::invalid_argument(boost::locale::translate("Not enough slopes"));
			}
			std::vector<RealX> ys(size);
			auto x_it = xs.begin();
			auto integral_it = ys.begin();
			auto integral = initial_value;
			*integral_it = integral;
			++integral_it;
			auto slope_it = slopes.begin();
			auto prev_x = *x_it;
			++x_it;
			for (; integral_it != ys.end(); ++integral_it, ++slope_it, ++x_it) {
				assert(slope_it != slopes.end());
				assert(x_it != xs.end());
				const auto next_x = *x_it;
				integral += (next_x - prev_x) * (*slope_it);
				*integral_it = integral;
				prev_x = next_x;
			}
			assert(integral_it == ys.end());
			assert(x_it == xs.end());
			return ys;
		}
	};
}
