#pragma once
#include "date_time.hpp"
#include "ip_address.hpp"

namespace simbo {
	/// Zredukowany punkt danych n6 w formacie binarnym.

	template <int Version> struct n6DataPoint {};

	/// Punkt danych (wersja 1).
	template <> struct n6DataPoint<1> {
		/// Typ adresu IP.
		typedef int64_t ip_t;

		/// Typ czasu.
		typedef uint32_t time_t;

		/// Typ kodu kraju.
		typedef uint8_t country_code_t;

		/// Konstruktor.
		n6DataPoint(seconds_t time, unsigned int count, seconds_t until, IpAddress ip, int country);

		n6DataPoint(n6DataPoint&&) = default;

		/// Adres IP bota w postaci liczbowej.
		ip_t ip;

		/// Pierwszy czas obserwacji w sekundach od Sinkhole::EPOCH.
		time_t time;

		/// Ostatni czas obserwacji w sekundach od Sinkhole::EPOCH.
		time_t until;

		/// Liczba obserwacji.
		uint16_t count;

		/// Kod kraju.
		country_code_t country;
	private:
		/// Wypełnienie (zawsze zero)
		int8_t zero_pad_[5];
	};
	static_assert(sizeof(n6DataPoint<1>) == 24, "Size of the n6DataPoint<1> struct is expected to be 24 bytes");
}