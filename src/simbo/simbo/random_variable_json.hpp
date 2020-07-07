#pragma once
#include "constant_random_variable.hpp"
#include "discrete_random_variable.hpp"
#include "exceptions.hpp"
#include "json.hpp"
#include "sparse_discrete_random_variable.hpp"

namespace simbo {
	// Funkcje do deserializacji implementacji RandomVariable<T> z formatu JSON.

	/// Deserializacja z JSON.
	/// Wymagane pola: type, params (jeżeli j jest obiektem JSON). Jeżeli j jest zwykłą wartością, spróbuj utworzyć ConstantRandomVariable<T>.
	/// \throw DeserialisationError Jeżeli pole "type" ma niespodziewaną wartość, albo brakuje któregoś z wymaganych pól.
	template <class T> void from_json(const json& j, std::unique_ptr<RandomVariable<T>>& rv) {
		if (j.is_object()) {
			validate_keys(j, "RandomVariable", { "type", "params" }, {});
			try {
				const std::string rv_type = j["type"];
				const json& j_params = j["params"];
				if (rv_type == "sparse_discrete") {
					deserialise_derived<SparseDiscreteRandomVariable<T>>(j_params, rv, "RandomVariable", rv_type);
				} else if (rv_type == "discrete") {
					deserialise_derived<DiscreteRandomVariable>(j_params, rv, "RandomVariable", rv_type);
				} else if (rv_type == "constant") {
					deserialise_derived<ConstantRandomVariable<T>>(j_params, rv, "RandomVariable", rv_type);
				} else {
					throw DeserialisationError("RandomVariable", j.dump(), (boost::locale::format("Unknown type: {1}") % rv_type).str());
				}
			} catch (DeserialisationError& e) {
				if (e.get_target_class() == "RandomVariable") {
					throw;
				} else {
					throw DeserialisationError("RandomVariable", j.dump(), e);
				}
			} catch (std::exception& e) {
				throw DeserialisationError("RandomVariable", j.dump(), e);
			}
		} else if (j.is_primitive()) {
			if (!j.is_null()) {
				/// Stała wartość.
				T v = j;
				rv = std::make_unique<ConstantRandomVariable<T>>(v);
			} else {
				rv = nullptr;
			}
		} else {
			throw DeserialisationError("RandomVariable", j.dump(), boost::locale::translate("JSON data must be primitive or object"));
		}
	}
}
