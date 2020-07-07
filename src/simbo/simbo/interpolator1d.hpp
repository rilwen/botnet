#pragma once
#include <vector>
#include <boost/locale.hpp>

namespace simbo {
	/**
	\brief Abstrakcyjny interpolator 1D.
	\tparam RealX Typ zmiennych X, reprezentujący liczby rzeczywiste (float, double...).
	\tparam RealY Typ zmiennych Y, reprezentujący liczby rzeczywiste (float, double...).
	Interpolator1D interpoluje funkcję zadaną w punktach x_1, ..., x_N o wartościach odpowiednio y_1, ..., y_N.
	*/
	template <class RealX, class RealY> class Interpolator1D {
	public:
		/// Wirtualny destruktor.
		virtual ~Interpolator1D() {}

		/// Domyślny konstruktor, wymagany przez dziedziczenie wirtualne tego interfejsu.
		Interpolator1D() {}

		/// Interpolator1D nie jest kopiowalny.
		Interpolator1D(const Interpolator1D<RealX, RealY>&) = delete;
		Interpolator1D<RealX, RealY>& operator=(const Interpolator1D<RealX, RealY>&) = delete;

		/// Dolny kraniec dziedziny.
		virtual RealX get_lower_bound() const = 0;

		/// Górny zakres dziedziny. Gwarantowane get_upper_bound() >= get_lower_bound().
		virtual RealX get_upper_bound() const = 0;

		/** \brief Interpolacja.
		\param x Argument.
		\return Wartość interpolowanej funkcji obliczona w punkcie x.
		\throws std::domain_error Jeżeli x < lower_bound() albo x > upper_bound().
		*/
		RealY operator()(RealX x) const {
			return evaluate(x, find_left_node_index(x));
		}

		/// Zwróć liczbę węzłów (> 0).
		virtual int size() const = 0;

		/**
		\brief Zwróć maksymalne i < size() t. że xs[i] <= x.
		\throws std::domain_error Jeżeli x < lower_bound() albo x > upper_bound().
		*/
		virtual int find_left_node_index(RealX x) const = 0;

		/**
		\brief Zwróć wartość Y odpowiadającą podanej wartości X.

		\param x Wartość X.
		\param left_node_index Indeks i najdalej położonego węzła xs[i] t. że xs[i] <= x. Wymagane (ale nie sprawdzane) aby left_node_index < size() oraz i >= 0.
		*/
		virtual RealY evaluate(RealX x, int left_node_index) const = 0;

		/** \brief Połączenie interpolacji z ekstrapolacją.
		\param x Argument.
		\return Wartość interpolowana, równa #operator()(x), jeżeli x mieści się w dziedzinie interpolatora, albo wartość ekstrapolowana płasko w przód jeżeli x > #get_upper_bound(), lub w tył jeżeli x < #get_lower_bound().
		*/
		RealY interpolate_and_extrapolate(const RealX x) const {
			if (x < get_lower_bound()) {
				return operator()(get_lower_bound());
			} else if (x > get_upper_bound()) {
				return operator()(get_upper_bound());
			} else {
				return operator()(x);
			}
		}
	};
}
