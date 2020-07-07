#pragma once
#include "public_connection.hpp"
#include "timestamped_value.hpp"

namespace simbo {
	class IpAddressPool;

	/** \brief Połączenie z dynamicznie przydzielanym adresem IP.
	*/
	class PublicConnectionDynamicAddress: public PublicConnection {
	public:
		typedef std::shared_ptr<IpAddressPool> ip_address_pool_ptr;

		/**
		\brief Konstruktor.
		\param ip_address_pool Pula adresów IP.
		\param max_address_lifetime Maksymalny czas życia adresu (w sekundach).
		\param creation_time Czas utworzenia połączenia.
		\throws std::domain_error Jeżeli max_address_lifetime <= 0, max_address_lifetime nie jest skończony albo creation_time nie jest skończony.
		\throws std::invalid_argument Jeżeli ip_address_pool == nullptr.
		*/
		PublicConnectionDynamicAddress(ip_address_pool_ptr ip_address_pool, seconds_t max_address_lifetime, seconds_t creation_time);
		
		void refresh_ip_address(seconds_t time_now) override;

		IpAddress get_ip_address() const override {
			return ip_address_.get_value();
		}

		seconds_t valid_from() const override {
			return ip_address_.get_time();
		}

		bool is_using_address_pool(const IpAddressPool& address_pool) const override;
	private:
		/// Zakres adresów IP.
		ip_address_pool_ptr ip_address_pool_;

		/// Adres IP.
		TimestampedValue<IpAddress> ip_address_;

		/// Maksymalny czas życia adresu.
		seconds_t max_address_lifetime_;
	};
}
