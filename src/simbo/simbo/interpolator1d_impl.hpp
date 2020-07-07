#pragma once
#include <boost/locale.hpp>
#include "interpolator1d.hpp"
#include "log.hpp"
#include "monotonicity.hpp"
#include "stl_formatters.hpp"
#include "utils.hpp"

namespace simbo {
	/// Podstawowa implementacja interfejsu Interpolator1D.
	template <class RealX, class RealY> class Interpolator1DImpl : public virtual Interpolator1D<RealX, RealY> {
	public:
		/**
		\brief Konstruktor.
		\param xs Współrzędne X węzłów. Przenoszone.
		\param ys Współrzędne Y węzłów. Przenoszone.
		\throws std::invalid_argument Jeżeli xs.empty(), ys.empty() albo xs.size() != ys.size(). Jeżeli wartości X nie są ściśle rosnące.
		*/
		Interpolator1DImpl(std::vector<RealX>&& xs, std::vector<RealY>&& ys) {
			if (xs.empty() || ys.empty() || xs.size() != ys.size()) {
				throw std::invalid_argument(boost::locale::translate("Xs and Ys must be of equal non-zero length"));
			}
			const auto monoton_xs = check_monotonicity(xs);
			if (xs.size() > 1 && monoton_xs != Monotonicity::STRICTLY_INCREASING) {
				if (get_logger()->level() <= spdlog::level::debug) {
					std::stringstream ss;
					ss << xs;
					get_logger()->debug("X values {} are not strictly increasing", ss.str());
				}				
				throw std::invalid_argument((boost::locale::format("X values are not strictly increasing: {1}") % monoton_xs).str());
			}
			xs_ = std::move(xs);
			ys_ = std::move(ys);
		}

		/// Dolny kraniec dziedziny.
		RealX get_lower_bound() const override {
			return xs_.front();
		}

		/// Górny zakres dziedziny. Gwarantowane get_upper_bound() >= get_lower_bound().
		RealX get_upper_bound() const override {
			return xs_.back();
		}

		/// Zwróć liczbę węzłów.
		int size() const override {
			return static_cast<int>(xs_.size());
		}

		/**
		\brief Zwróć maksymalne i < size() t. że xs[i] <= x.
		\throws std::domain_error Jeżeli x < lower_bound() albo x > upper_bound().
		*/
		int find_left_node_index(RealX x) const override {
			return find_left_node_index(xs_.begin(), xs_.end(), x);
		}

		/// Zwróć stałą referencję do wektora X.
		const std::vector<RealX>& get_xs() const {
			return xs_;
		}

		/// Zwróć stałą referencję do wektora Y.
		const std::vector<RealY>& get_ys() const {
			return ys_;
		}
	protected:
		/**
		\brief Zwróć index i największego węzła x[i] takiego że x[i] <= value.
		\return Indeks węzła.
		\throw std::invalid_argument Jeżeli begin == end
		\throw std::domain_error Jeżeli value < *begin albo value > *(end - 1).
		*/
		template <class It, class V> static int find_left_node_index(const It begin, const It end, const V& value) {
			const auto it = find_left_node(begin, end, value);
			assert(begin != end);
			if (value > *(end - 1)) {
				throw std::domain_error(boost::locale::translate("Value above upper bound"));
			}
			return static_cast<int>(std::distance(begin, it));
		}

		/// Konstruktor kopiujący.
		Interpolator1DImpl(const Interpolator1DImpl<RealX, RealY>& other)
			: xs_(other.xs_), ys_(other.ys_) {}

		/// Operator przypisania przez przenoszenie.
		Interpolator1DImpl<RealX, RealY>& operator=(Interpolator1DImpl<RealX, RealY>&& other) {
			if (this != &other) {
				xs_ = std::move(other.xs_);
				ys_ = std::move(other.ys_);
			}
			return *this;
		}
	private:
		/// Wartości X węzłów.
		std::vector<RealX> xs_;

		/// Wartości Y węzłów.
		std::vector<RealY> ys_;
	};
}
