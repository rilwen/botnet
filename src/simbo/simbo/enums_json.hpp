#pragma once
#include <typeinfo>
#include <type_traits>
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {

	// Funkcje konwertujące nasze typy enum do / z formatu JSON.

	/// Konwertuj enum do formatu JSON.
	/// \param j Dane JSON.
	/// \param value Wartość enum.
	/// \tparam E Typ enum. Zakładamy, że odpowiada on wartościom całkowitym 0, 1, 2, ... N - 1.
	template <class E> std::enable_if_t<std::is_enum_v<E>, void> to_json(json& j, E value) {
		j = to_string(value);
	}

	/// Konwertuj enum z formatu JSON.
	/// \param j Dane JSON.
	/// \param value Referencja do wartości enum.
	/// \tparam E Typ enum. Zakładamy, że odpowiada on wartościom całkowitym 0, 1, 2, ... N - 1.
	/// \throw DeserialisationError Jeżeli konwersja zawiedzie.
	template <class E> std::enable_if_t<std::is_enum_v<E>, void> from_json(const json& j, E& value) {
		try {
			from_string(j.get<std::string>(), value);
		} catch (std::exception& e) {
			throw DeserialisationError(typeid(E).name(), j.dump(), e);
		}
	}
}
