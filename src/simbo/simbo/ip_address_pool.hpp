#pragma once
#include <gtest/gtest_prod.h>
#include "ip_address.hpp"

namespace simbo {
	/// Zakres adresów IP.
	class IpAddressPool {
	public:
		/// Wirtualny destruktor.
		virtual ~IpAddressPool() {}

		/// Najniższy adres w zakresie.
		virtual IpAddress min() const = 0;

		/// Najwyższy adres w zakresie.
		virtual IpAddress max() const = 0;

		/// Całkowita liczba adresów w zakresie.
		virtual int64_t total_size() const = 0;

		/// Liczba przydzielonych adresów.
		virtual int64_t allocated_size() const = 0;

		/// Alokuj nowy adres.
		/// \throws std::logic_error Jeżeli allocated_size() == total_size().
		virtual IpAddress allocate() = 0;

		/// Zwróć adres do puli.
		/// \throws std::invalid_argument Jeżeli adres released nie jest adresem z tej puli, albo jeżeli nie był zarezerwowany.
		virtual void release(const IpAddress& released) = 0;

		/// Zwróć adres do puli i alokuj nowy.
		virtual IpAddress update(const IpAddress& released) {
			release(released);
			return allocate();
		}
	protected:
		/// Podbij adres w bytes o dodatnie delta.
		/// \throws std::domain_error Jeżeli zajdzie overflow.
		FRIEND_TEST(IpAddressPoolTest, advance);
		template <size_t N> static void advance(std::array<unsigned char, N>& bytes, int64_t delta);
	};

	template <size_t N> void IpAddressPool::advance(std::array<unsigned char, N>& bytes, int64_t delta) {
		assert(delta >= 0);
		// Dodawanie w słupku.
		int r = 0;
		for (int i = static_cast<int>(N - 1); i >= 0; --i) {
			assert(r <= 1);
			assert(r >= 0);
			const int d = delta % 256;
			int s = bytes[i] + d + r;
			if (s > 255) {
				assert(s < 512);
				r = 1;
				s = s - 256;
			} else {
				r = 0;
			}
			bytes[i] = static_cast<unsigned char>(s);
			delta = (delta - d) / 256;
		}
		if (r != 0 || delta != 0) {
			throw std::domain_error(boost::locale::translate("Overflow while adding bytes arrays"));
		}
	}
}
