#pragma once
#include "enums.hpp"

namespace simbo {
	/// Typ sieci lokalnej.
	enum class LocalNetworkType {
		/// Domowa.
		HOUSEHOLD,

		/// Firmowa.
		CORPORATE
	};

	template <> struct EnumNames<LocalNetworkType> {
		static constexpr std::array<const char*, 2> NAMES = {
			"HOUSEHOLD",
			"CORPORATE"
		};
	};
}
