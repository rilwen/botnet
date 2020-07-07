#pragma once
#include <memory>
#include "id_factory.hpp"
#include "laptop.hpp"

namespace simbo {
	class Desktop;
	class LocalNetwork;
	class OperatingSystem;
	class Server;
	class Sysadmin;

	/// Fabryka obiektów klasy Host.
	class HostFactory {
	public:
		// Skrócone nazwy typów.		
		typedef Host::operating_system_ptr operating_system_ptr;
		typedef IdFactory::id_t id_t;

		/// Stwórz host klasy Desktop i przekaż obiektowi sysadmin.
		/// \param sysadmin Zobacz Host::Host
		/// \param os Zobacz Host::Host
		/// \param always_on Zobacz Host::Host
		/// \param local_network Zobacz Desktop::Desktop
		/// \throws std::runtime_error Jeżeli wyczerpały się numery ID.
		/// \throws Zobacz Desktop::Desktop.
		Desktop* make_desktop(Sysadmin& sysadmin, operating_system_ptr&& os, bool always_on, LocalNetwork* local_network);

		/// Stwórz host klasy Server i przekaż obiektowi sysadmin.
		/// \param sysadmin Zobacz Host::Host
		/// \param os Zobacz Host::Host
		/// \param local_network Zobacz Server::Server
		/// \throws std::runtime_error Jeżeli wyczerpały się numery ID.
		/// \throws Zobacz Server::Server.
		Server* make_server(Sysadmin& sysadmin, operating_system_ptr&& os, LocalNetwork* local_network);

		/// Stwórz host klasy Laptop i przekaż obiektowi sysadmin.
		/// \param sysadmin Zobacz Host::Host
		/// \param os Zobacz Host::Host
		/// \param local_network_switcher Zobacz Desktop::Desktop
		/// \throws std::runtime_error Jeżeli wyczerpały się numery ID.
		/// \throws Zobacz Laptop::Laptop.
		Laptop* make_laptop(Sysadmin& sysadmin, operating_system_ptr&& os, Laptop::local_network_switcher_t&& local_network_switcher);
	private:
		/// Fabryka numerów ID.
		IdFactory id_factory_;
	};
}
