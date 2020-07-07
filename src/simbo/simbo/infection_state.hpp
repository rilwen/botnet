#pragma once
#include "enums.hpp"

namespace simbo {
	/// Status infekcji hosta.
	enum class InfectionState {
		/// Jeszcze nie zainfekowany.
		NOT_INFECTED,

		/// Zainfekowany.
		INFECTED
	};

	template <> struct EnumNames<InfectionState> {
		static constexpr std::array<const char*, 2> NAMES = {
			"NOT_INFECTED",
			"INFECTED"
		};
	};
}