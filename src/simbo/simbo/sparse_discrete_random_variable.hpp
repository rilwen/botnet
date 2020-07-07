#pragma once
#include <set>
#include "discrete_random_variable.hpp"
#include "exceptions.hpp"
#include "json.hpp"
#include "random_variable.hpp"

namespace simbo {
	/** \brief Zmienna losowa przyjmująca wartości X = x_k z prawdopodobieństwami p_k = P(X = x_k).
	*/
	template <class T> class SparseDiscreteRandomVariable : public RandomVariable<T> {
	public:
		/** \brief Konstruktor.
		\param weights Wektor wag (klasa normuje je do prawdopodobieństw sumujących się do 1).
		\param values Wektor unikalnych wartości (przenoszony).
		\throw std::invalid_argument Jeżeli weights.empty() albo jeżeli suma wag jest zero. Jeżeli któraś waga w_i nie spełnia warunku w_i >= 0. Jeżeli weights.size() != values.size(). Jeżeli jakaś wartość pojawia się w values więcej niż raz.
		*/
		SparseDiscreteRandomVariable(const std::vector<double>& weights, std::vector<T>&& values);

		/** \brief Konstruktor zmiennej stałej.
		\param value Stała wartość.
		*/
		SparseDiscreteRandomVariable(const T& value);

		/// Zwróć wektor możliwych wartości.
		const std::vector<T>& get_values() const {
			return values_;
		}

		/// Zwróć liczbę wartości.
		int get_number_values() const {
			return static_cast<int>(values_.size());
		}

		T operator()(RNG& rng) const override {
			return values_[index_variable_(rng)];
		}

		/** \brief Zwróć prawdopodobieństwo wartości.
		\param idx Indeks wartości.
		\throw std::out_of_range Jeżeli idx < 0 albo idx >= #get_number_values().
		*/
		double get_probability(int idx) const {
			try {
				return index_variable_.get_probability(idx);
			} catch (std::domain_error&) {
				throw std::out_of_range((boost::locale::format("Value index out of range: {1}") % idx).str());
			}
		}

		/** \brief Zwróć wartość.
		\param idx Indeks wartości.
		\throw std::out_of_range Jeżeli idx < 0 albo idx >= #get_number_values().
		*/
		T get_value(int idx) const {
			if (idx < 0 || idx >= get_number_values()) {
				throw std::out_of_range((boost::locale::format("Value index out of range: {1}") % idx).str());
			}
			return values_[idx];
		}
	private:
		std::vector<T> values_;
		DiscreteRandomVariable index_variable_;
	};

	template <class T> SparseDiscreteRandomVariable<T>::SparseDiscreteRandomVariable(const std::vector<double>& weights, std::vector<T>&& values)
		: index_variable_(weights, 0) {
		if (values.size() != weights.size()) {
			throw std::invalid_argument(boost::locale::translate("Weights and values vectors have different sizes"));
		}
		std::set<T> unique_values(values.begin(), values.end());
		if (unique_values.size() != values.size()) {
			throw std::invalid_argument(boost::locale::translate("Some values appear more than once"));
		}
		values_ = std::move(values);
	}

	template <class T> SparseDiscreteRandomVariable<T>::SparseDiscreteRandomVariable(const T& value)
		: SparseDiscreteRandomVariable(std::vector<double>({ 1. }), std::vector<T>({ value })) {}

	/// Deserializacja z JSON.
	/// Wymagane pola: weights, values.
	/// \throw DeserialisationError Jeżeli deserializacja się nie powiedzie.
	template <class T> void from_json(const json& j, std::unique_ptr<SparseDiscreteRandomVariable<T>>& rv) {
		validate_keys(j, "SparseDiscreteRandomVariable", { "weights" , "values" }, {});
		try {
			std::vector<double> weights = j["weights"];
			std::vector<T> values = j["values"];
			rv = std::make_unique<SparseDiscreteRandomVariable<T>>(weights, std::move(values));
		} catch (std::exception& e) {
			throw DeserialisationError("SparseDiscreteRandomVariable", j.dump(), e);
		}
	}

	/// Deserializacja z JSON.
	/// Wymagane pola: weights, values.
	/// \throw DeserialisationError Jeżeli deserializacja się nie powiedzie.
	template <class T> void from_json(const json& j, std::shared_ptr<SparseDiscreteRandomVariable<T>>& rv) {
		std::unique_ptr<SparseDiscreteRandomVariable<T>> ptr;
		from_json(j, ptr);
		rv = std::move(ptr);
	}

	/** Deserializacja z JSON.
	Wymagane pola: weights, values.
	\param j Dane JSON.
	\param aux Pomocniczy obiekt do deserializacji wartości.
	\param[out] rv Wskaźnik do nowego obiektu.
	\tparam Aux Typ pomocniczego obiektu.
	\throw DeserialisationError Jeżeli deserializacja się nie powiedzie.
	*/
	template <class Aux, class T> void from_json(const json& j, const Aux& aux, std::unique_ptr<SparseDiscreteRandomVariable<T>>& rv) {
		validate_keys(j, "SparseDiscreteRandomVariable", { "weights" , "values" }, {});
		try {
			std::vector<double> weights = j["weights"];
			const json& j_values = j["values"];
			std::vector<T> values;
			if (j_values.is_array()) {
				values.reserve(j_values.size());
				for (const json& jv : j_values) {
					T v;
					from_json(jv, aux, v);					
					values.push_back(std::move(v));
				}
			} else {
				throw DeserialisationError("SparseDiscreteRandomVariable", j.dump(), "Expecting \"values\" to be an array");
			}
			rv = std::make_unique<SparseDiscreteRandomVariable<T>>(weights, std::move(values));
		} catch (std::exception& e) {
			throw DeserialisationError("SparseDiscreteRandomVariable", j.dump(), e);
		}
	}

	/// Serializacja do JSON.
	template <class T> void to_json(json& j, const std::shared_ptr<SparseDiscreteRandomVariable<T>> rv) {
		if (!rv) {
			throw std::invalid_argument(boost::locale::translate("Null pointer"));
		}
		const int n = rv->get_number_values();
		std::vector<double> weights(n);
		for (int i = 0; i < n; ++i) {
			weights[i] = rv->get_probability(i);
		}
		j = {
			{"weights", weights},
			{"values", rv->get_values()}
		};
	}
}
