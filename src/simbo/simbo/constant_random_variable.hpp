#pragma once
#include "json.hpp"
#include "random_variable.hpp"

namespace simbo {

	/**
	\brief Stała zmienna losowa.

	Szablon modeluje zmienną losową o stałej wartości.

	\tparam T Typ wartości zmiennej losowej.
	*/
	template <class T> class ConstantRandomVariable: public RandomVariable<T> {
	public:
		/// Konstruktor kopiujący wartość.
		/// \param value Stała wartość.
		ConstantRandomVariable(const T& value)
			: value_(value) {}

		T operator()(RNG&) const override {
			return value_;
		}
	private:
		T value_;
	};

	/// Deserializacja z JSON.
	/// Wymagane pole: value.
	/// \throw std::DeserialisationError Jeżeli deserializacja się nie powiedzie.
	template <class T> void from_json(const json& j, std::unique_ptr<ConstantRandomVariable<T>>& rv) {
		validate_keys(j, "SparseDiscreteRandomVariable", { "value" }, {});
		try {
			T v = j["value"];
			rv = std::make_unique<ConstantRandomVariable<T>>(v);
		} catch (std::exception& e) {
			throw DeserialisationError("ConstantRandomVariable", j.dump(), e);
		}
	}
}
