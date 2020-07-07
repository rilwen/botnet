#pragma once
#include <memory>
#include <boost/dynamic_bitset.hpp>
#include "ip_address_pool.hpp"

namespace simbo {
	/// \brief Pula adresów przydzielająca dane IP tylko jednemu klientowi.
	/// \tparam Addr Klasa adresu: boost::asio::ip::address_v4 albo boost::asio::ip::address_v6.
	template <class Addr> class UniqueIpAddressPool: public IpAddressPool {
	public:
		typedef Addr addr_t;
		typedef typename Addr::bytes_type bytes_t;

		/** \brief Skonstruuj zakres adresów [min, max].

		\param min Najniższy adres.
		\param max Najwyższy adres.
		\throws std::invalid_argument Jeżeli min > max. 
		\throws std::range_error Jeżeli rozmiar zakresu adresów przepełniłby zakres typu int64.
		*/
		UniqueIpAddressPool(const addr_t& min, const addr_t& max);

		IpAddress min() const override {
			return min_;
		}

		IpAddress max() const override {
			return max_;
		}

		int64_t total_size() const override {
			return size_;
		}

		int64_t allocated_size() const override {
			return allocated_size_;
		}

		IpAddress allocate() override;

		void release(const IpAddress& released) override;
	private:
		// Kolejność deklaracji jest istotna.

		/// Najniższy adres w zakresie.
		IpAddress min_;

		/// Najwyższy adres w zakresie.
		IpAddress max_;

		/// Najniższy adres w zakresie (jako liczba całkowita).
		bytes_t min_as_bytes_;

		/// Liczba adresów w zakresie.
		int64_t size_;

		/// Przechowuje informację o tym, czy i-ty adres z zakresu jest dostępny.
		boost::dynamic_bitset<> is_available_;

		/// Liczba przydzielonych aktualnie adresów.
		int64_t allocated_size_;
	};

	// Aliasy typów.
	
	/// Pula unikatowych adresów IPv4.
	typedef UniqueIpAddressPool<boost::asio::ip::address_v4> UniqueIpAddressPoolV4;
	
	/// Pula unikatowych adresów IPv6.
	typedef UniqueIpAddressPool<boost::asio::ip::address_v6> UniqueIpAddressPoolV6;

	/// Zwróć UniqueIpAddressPoolV4 albo UniqueIpAddressPoolV6, zależnie od typu min i max.
	/// \throws std::invalid_argument Jeżeli min i max są różnego typu. Zobacz również UniqueIpAddressPool::UniqueIpAddressPool. 
	std::unique_ptr<IpAddressPool> make_unique_ip_address_pool(const IpAddress& min, const IpAddress& max);
}
