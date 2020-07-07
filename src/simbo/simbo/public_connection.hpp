#pragma once
#include "date_time.hpp"
#include "ip_address.hpp"
#include "managed_object.hpp"

namespace simbo {
	
	class InternetServiceProvider;
	class IpAddressPool;

	/**
	\brief Aktywne połączenie sieci lokalnej z resztą świata.

	Dzielony pomiędzy jedną lub więcej sieciami lokalnymi. Należy i jest zarządzany przez dostawcę usług internetowych (ISP).
	Połączenie jest tworzone przez ISP na życzenie klienta.

	*/
	class PublicConnection: public ManagedObject<PublicConnection, InternetServiceProvider> {
	public:
		/// Wirtualny destruktor.
		virtual ~PublicConnection() {}

		/**
		\brief Odśwież adres IP.
		\param time_now Aktualny czas.		
		\throws std::domain_error Jeżeli time_now <= #valid_from().
		*/
		virtual void refresh_ip_address(seconds_t time_now) = 0;

		/// Zwróć aktualny adres IP.
		/// \returns Adres IP.
		virtual IpAddress get_ip_address() const = 0;

		/// Czas od którego jest ważna aktualna konfiguracja łącza.		
		virtual seconds_t valid_from() const = 0;

		/// Sprawdź czy połączenie korzysta z danej puli adresów.
		virtual bool is_using_address_pool(const IpAddressPool& address_pool) const = 0;
	};
}
