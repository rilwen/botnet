#pragma once
#include "enums.hpp"

namespace simbo {
	/// Typ hosta.
	enum class HostType {
		DESKTOP,
		
		LAPTOP,

		SERVER
	};

	template <> struct EnumNames<HostType> {
		static constexpr std::array<const char*, 3> NAMES = {
			"DESKTOP",
			"LAPTOP",
			"SERVER"
		};
	};

	/// Czy host o tym typie jest stały.
	bool is_host_type_fixed(HostType host_type);
}
