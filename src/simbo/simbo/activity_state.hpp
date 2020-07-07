#pragma once
#include "enums.hpp"

namespace simbo {
	/// Stan aktywności hosta albo innego systemu.
	/// 
	enum class ActivityState {
		/// Host jest wyłączony.
		OFF = 0,

		/// Host jest włączony.
		ON = 1
	};

	template <> struct EnumNames<ActivityState> {
		static constexpr std::array<const char*, 2> NAMES = {
			"OFF",
			"ON"
		};
	};
}
