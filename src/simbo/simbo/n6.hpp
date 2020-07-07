#pragma once
#include <cstdint>
#include <iosfwd>
#include "n6_data_point.hpp"

namespace simbo {
	/// Funkcje służące do konwersji danych JSON z n6 do formatu binarnego.
	namespace n6 {
		typedef n6DataPoint<1> data_point_t;

		/// Data od której liczone są sekundy.
		extern const DateTime EPOCH;

		/// Nagłówek formatu binarnego.
		struct Header {
			Header(size_t number_rows);
			static const int reserved_size = 96;
			int8_t marker[8] = { 'N', '6', 'B', 'I', 'N', 0, 0, 0 };
			uint64_t version = 1;
			uint64_t row_size = sizeof(data_point_t);
			uint64_t number_rows;
			int8_t reserved[reserved_size];
		};

		/// Zapisz nagłówek do pliku binarnego.
		/// \param number_rows Liczba wierszy danych.
		/// \param out Strumień wyjścia do pliku binarnego.
		void write_header(size_t number_rows, std::ostream& out);
	}
}