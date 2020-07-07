#pragma once
#include "enums.hpp"

namespace simbo {
	/// Format zapisu danych.
	enum class DataStorageFormat {
		CSV, /// Format tekstowy.
		BIN,  /// Format binarny Simbo.
		NONE  /// Brak zapisu.
	};

	template <> struct EnumNames<DataStorageFormat> {
		static constexpr std::array<const char*, 3> NAMES = {
			"CSV",
			"BIN",
			"NONE"
		};
	};

	/// Format pliku odpowiadający rozszerzeniu.
	/// \param extension "csv" dla CSV albo "bin" dla BIN. Inne wartości zabronione.
	/// \throw std::invalid_argument Jeżeli rozszerzenie jest niedozwolone.
	DataStorageFormat data_storage_format_from_extension(const std::string& extension);
}
