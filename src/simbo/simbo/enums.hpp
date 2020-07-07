#pragma once
#include <array>
#include <iosfwd>
#include <typeinfo>
#include <type_traits>
#include "exceptions.hpp"

namespace simbo {
	// Funkcje operujące na typach enum.
	// Funkcje konwertujące łańcuchy znaków w danych JSON na klasy enum są zdefiniowane w enums_json.hpp.

	/// Dla każdego typu enum / enum class E struktura EnumNames powinna deklarować statyczną tablicę NAMES zawierającą wartości const char* z ciągami znaków odpowiadającymi kolejnym wartościom typu E.
	template <class E> struct EnumNames {
		static_assert(std::is_enum_v<E>, "E must be an enum type");
		static constexpr std::array<const char*, 0> NAMES = {};
	};

	/// Konwertuj enum do wartości tekstowej.
	/// \param value Wartość enum.
	/// \tparam E Typ enum. Zakładamy, że odpowiada on wartościom całkowitym 0, 1, 2, ... N - 1.
	/// \return Wskaźnik do stałej tekstowej odpowiadającej value.
	template <class E> constexpr std::enable_if_t<std::is_enum_v<E>, const char*> to_string(E value) {
		return EnumNames<E>::NAMES[static_cast<size_t>(value)];
	}

	/// Konwertuj wartość tekstowa do enum.
	/// \param str Wartość tekstowa.
	/// \param value Referencja która zostanie ustawiona na właściwą wartość.
	/// \tparam E Typ enum. Zakładamy, że odpowiada on wartościom całkowitym 0, 1, 2, ... N - 1.
	/// \throw std::invalid_argument Jeżeli str nie odpowiada żadnej wartości enum.
	template <class E> std::enable_if_t<std::is_enum_v<E>, void> from_string(const std::string& str, E& value) {
		static_assert(EnumNames<E>::NAMES.size() > 0, "EnumNames<E>::NAMES should not be empty");
		for (size_t i = 0; i < EnumNames<E>::NAMES.size(); ++i) {
			if (str == EnumNames<E>::NAMES[i]) {
				value = static_cast<E>(i);
				return;
			}
		}
		throw std::invalid_argument((boost::locale::format("Unknown {1}: {2}") % typeid(E).name() % str).str());
	}	
}

namespace std {
	template <class E> std::enable_if_t<std::is_enum_v<E>, std::ostream&> operator<<(std::ostream& os, E value) {
		os << simbo::to_string<E>(value);
		return os;
	}
}
