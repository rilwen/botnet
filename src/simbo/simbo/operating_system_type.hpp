#pragma once
#include <utility>
#include "enums.hpp"

namespace simbo {
	/// Rodzaj systemu operacyjnego.
	enum class OperatingSystemType {
		ANDROID,
		LINUX,
		MACOS,
		WINDOWS,
		OTHER
	};

	template <> struct EnumNames<OperatingSystemType> {
		static constexpr std::array<const char*, 5> NAMES = {
			"ANDROID",
			"LINUX",
			"MACOS",
			"WINDOWS",
			"OTHER"
		};
	};
}
