#pragma once
#include "fixed_host.hpp"
#include "passkey.hpp"

namespace simbo {
	class HostFactory;

	/// Komputer desktop (stacja robocza, domowy PC, itd.). 
	class Desktop : public FixedHost {
	public:
		/** \brief Konstruktor. Wołany z wewnątrz klasy HostFactory.
		\param id Zobacz FixedHost::FixedHost
		\param operating_system Zobacz FixedHost::FixedHost
		\param always_on Zobacz FixedHost::FixedHost
		\param local_network Sieć do której komputer jest podłączony na stałe.
		\param key Klucz z HostFactory.
		\throws Zobacz FixedHost::FixedHost
		*/
		Desktop(id_t id, operating_system_ptr&& operating_system, bool always_on, LocalNetwork* local_network, Passkey<HostFactory> key)
			: FixedHost(id, std::move(operating_system), always_on, local_network) {
		}

		HostType get_type() const override {
			return HostType::DESKTOP;
		}
	};
}
