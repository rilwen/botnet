#pragma once
#include <boost/asio/ip/address.hpp>
#include <boost/locale.hpp>
#include "json_fwd.hpp"

namespace simbo {
	/// Implementacja adresu IP (v4 albo v6).
	class IpAddress : public boost::asio::ip::address {
	public:
		using boost::asio::ip::address::address;

		/// Domyślny konstruktor.
		IpAddress();
		
		/// Skonstruuj z klasy macierzystej.
		IpAddress(const boost::asio::ip::address& addr)
			: boost::asio::ip::address(addr) {}

		IpAddress(const IpAddress&) = default;

		/// Przypisz obiekt klasy macierzystej.
		IpAddress& operator=(const boost::asio::ip::address& addr) {
			boost::asio::ip::address::operator=(addr);
			return *this;
		}

		IpAddress& operator=(const IpAddress&) = default;

		static IpAddress from_string(const std::string& str) {
			return boost::asio::ip::make_address(str);
		}

		/**
		\brief Skonwertuj adres do liczby całkowitej.
		\param address_bytes Adres IP jako tablica bajtow w uporządkowaniu sieciowym.
		\throws std::range_error Jeżeli konwersja przepełniłaby zakres typu int64_t.
		\returns Liczba nieujemna.
		*/
		template <size_t N> static int64_t to_int64(const std::array<unsigned char, N>& address_bytes);

		/**
		\brief Skonwertuj adres do liczby całkowitej.
		\throws std::range_error Jeżeli konwersja przepełniłaby zakres typu int64_t.
		\returns Liczba nieujemna.
		*/
		int64_t to_int64() const;

		/**
		\brief Zwróć Odległość adr1 od adr2.
		\param adr1 Pierwszy adres.
		\param adr2 Drugi adrres.
		\throws std::invalid_argument Jeżeli adresy są różnego typu 
		\throws std::range_error Jeżeli odległość adresów przepełniłaby zakres typu int64_t.
		\returns Liczba nieujemna.
		*/
		static int64_t distance(const IpAddress& adr1, const IpAddress& adr2);

		/**
		\brief Zwróć liczbę adresów w przedziale [min, max]. Adresy muszą być tego samego typu.
		\param min Najniższy adres.
		\param max Najwyższy adres.
		\throws std::invalid_argument Jeżeli min > max albo adresy są różnego typu.
		\throws std::range_error Jeżeli liczba adresów w zakresie przepełniłaby zakres typu int64_t.
		\returns Liczba dodatnia.
		*/
		static int64_t range_size(const IpAddress& min, const IpAddress& max);

		/// Sprawdź czy adresy są tego samego typu.
		static bool same_type(const IpAddress& adr1, const IpAddress& adr2);

	private:
		/// Oblicz a - b dla a >= b, gdzie a i b są liczbami całkowitymi zapisanymi jako tablice bajtów.
		/// \throws std::invalid_argument Jeżeli a < b.
		template <size_t N> static std::array<unsigned char, N> sub(const std::array<unsigned char, N>& a, const std::array<unsigned char, N>& b);
	};

	template <size_t N> int64_t IpAddress::to_int64(const std::array<unsigned char, N>& address_bytes) {
		if (N > 8) {
			for (size_t i = 0; i < N - 8; ++i) {
				if (address_bytes[i]) {
					throw std::range_error(boost::locale::translate("Conversion to int64_t will overflow"));
				}
			}
		} else if (N == 8 && address_bytes[0] > 127) {
			throw std::range_error(boost::locale::translate("Conversion to int64_t will overflow"));
		}
		int64_t value = 0;
		for (size_t i = 0; i < N; ++i) {
			value = value * 256 + address_bytes[i];
		}
		assert(value >= 0);
		return value;
	}

	/** \brief Odczytaj adres z formatu JSON.
	\throw DeserialisationError Jeżeli konwersja zawiedzie.
	*/
	void from_json(const json& j, IpAddress& ip_address);
}
