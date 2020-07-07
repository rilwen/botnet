#pragma once
#include <vector>
#include "json_fwd.hpp"
#include "random_variable.hpp"

namespace simbo {
	/// Zmienna losowa o wartościach dyskretnych.
	class DiscreteRandomVariable : public RandomVariable<int> {
	public:
		/// Domyślny konstruktor, tworzy zmienną równą zawsze zero.
		DiscreteRandomVariable();

		/** Konstruktor.
		\param weights Wagi kolejnych wartości. \f$ P(k) = \mathrm{weights}_k / \sum_l \mathrm{weights}_l \f$. Wagi muszą być nieujemne.
		\param min Minimalna wartość.
		\throws std::invalid_argument Jeżeli weights.empty() albo jeżeli suma wag jest zero. Jeżeli któraś waga w_i nie spełnia warunku w_i >= 0.
		*/
		DiscreteRandomVariable(const std::vector<double>& weights, int min);

		/// Konstruktor kopiujący.
		DiscreteRandomVariable(const DiscreteRandomVariable& other);

		/// Konstruktor przenoszący.
		DiscreteRandomVariable(DiscreteRandomVariable&& other);

		/// Operator przypisania.
		DiscreteRandomVariable& operator=(const DiscreteRandomVariable& other);

		int operator()(RNG& rng) const override;

		/// Zwróć minimalną wartość zmiennej.
		int min() const {
			return min_;
		}

		/// Zwróć maksymalną wartość zmiennej.
		int max() const {
			return max_;
		}

		/** \brief Zwróć prawdopodobieństwo wylosowania i.
		\param i Wartość.
		\throw std::domain_error Jeżeli i < #min() albo i > #max().
		*/
		double get_probability(int i) const;
	private:
		/// Dystrybuanta.
		std::vector<double> cdf_;

		/// Wartość minimalna.
		int min_;

		/// Wartość maksymalna.
		int max_;
	};

	/// Deserializacja z JSON.
	/// Wymagane pola: weights, min.
	void from_json(const json& j, std::unique_ptr<DiscreteRandomVariable>& rv);
}
