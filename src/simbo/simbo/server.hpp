#pragma once
#include "fixed_host.hpp"
#include "passkey.hpp"

namespace simbo {
	class HostFactory;

	/// Server. Włączony cały czas.
	class Server : public FixedHost {
	public:
		/** \brief Konstruktor. Wołany z wewnątrz klasy HostFactory.
		\param id Zobacz FixedHost::FixedHost
		\param operating_system Zobacz FixedHost::FixedHost
		\param local_network Sieć do której komputer jest podłączony na stałe.
		\param key Klucz z HostFactory.
		\throws Zobacz FixedHost::FixedHost
		*/
		Server(id_t id, operating_system_ptr&& operating_system, LocalNetwork* local_network, Passkey<HostFactory> key)
			: FixedHost(id, std::move(operating_system), true, local_network) {
		}

		HostType get_type() const override {
			return HostType::SERVER;
		}
	};
}
