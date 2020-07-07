#pragma once
#include <memory>
#include <vector>
#include "object_manager.hpp"
#include "public_connection.hpp"

namespace simbo {

	class IpAddressPool;

	/// Dostawca usług internetowych.
	class InternetServiceProvider: private ObjectManager<PublicConnection, InternetServiceProvider> {
	public:
		typedef std::unique_ptr<PublicConnection> connection_ptr;

		/** \brief Konstruktor.
		\param ip_address_pool Pula adresów IP obsługiwana przez ISP.
		\throw std::invalid_argument Jeżeli ip_address_pool == nullptr.
		*/
		InternetServiceProvider(std::shared_ptr<IpAddressPool> ip_address_pool);

		/// Dodaj nowe połączenie.
		/// \param connection Wskaźnik do dodawanego połączenia.
		/// \throws std::invalid_argument Jeżeli connection == nullptr albo nie korzysta z puli adresowej ISP.
		void add_connection(connection_ptr&& connection);

		/// Odśwież adresy IP obsługiwanych połączeń. Nie null.
		void refresh_ip_addresses(seconds_t now) const;

		/// Zwróć referencję do puli adresów.
		IpAddressPool& get_address_pool() {
			return *ip_address_pool_;
		}
	private:
		std::shared_ptr<IpAddressPool> ip_address_pool_;
	};
}
