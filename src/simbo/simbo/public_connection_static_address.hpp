#pragma once
#include <cassert>
#include "public_connection.hpp"

namespace simbo {

	/// Połączenie ze stałym adresem IP zarezerwowanym od początku do końca symulacji.
	class PublicConnectionStaticAddress: public PublicConnection {
	public:
		/**
		Konstruktor.
		\param ip_address Adres IP.	
		\throws std::invalid_argument Jeżeli adres IP jest nieokreślony.
		*/
		PublicConnectionStaticAddress(IpAddress ip_address);

		void refresh_ip_address(seconds_t time_now) override {
			assert(time_now > valid_from());
		}

		IpAddress get_ip_address() const override {
			return ip_address_;
		}

		seconds_t valid_from() const override {
			return -infinite_time();
		}

		bool is_using_address_pool(const IpAddressPool& address_pool) const override;
	private:
		IpAddress ip_address_;		
	};
}
