#pragma once
#include "enums.hpp"

namespace simbo {
	/// Środowisko pracy / zabawy użytkownika.
	enum class UserEnvironmentType {
		/// Dom.
		HOME,

		/// Biuro.
		OFFICE,

		/// Żadne (nie używa).
		NONE
	};

	template <> struct EnumNames<UserEnvironmentType> {
		static constexpr std::array<const char*, 3> NAMES = {
			"HOME",
			"OFFICE",
			"NONE"
		};
	};
}
