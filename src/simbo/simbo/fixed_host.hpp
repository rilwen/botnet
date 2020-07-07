#pragma once
#include "host.hpp"

namespace simbo {
	/// Host niemobilny. Na stałe podłączony do tej samej sieci lokalnej.
	class FixedHost : public Host {
	public:
		const_local_network_ptr get_local_network() const override {
			return local_network_;
		}

		bool is_fixed() const override {
			return true;
		}
	protected:
		/** \brief Konstruktor. Wołany z podklas.
		\param id Zobacz Host::Host
		\param operating_system Zobacz Host::Host
		\param always_on Zobacz Host::Host
		\param local_network Sieć do której komputer jest podłączony na stałe.
		\throws Zobacz Host::Host
		\throws std::invalid_argument Jeżeli local_network == nullptr.
		*/
		FixedHost(id_t id, operating_system_ptr&& operating_system, bool always_on, local_network_ptr local_network);
	private:
		const_local_network_ptr local_network_;
	};
}
