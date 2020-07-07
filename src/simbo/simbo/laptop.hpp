#pragma once
#include <memory>
#include <vector>
#include "const_view.hpp"
#include "markov_switcher.hpp"
#include "mobile_host.hpp"
#include "passkey.hpp"

namespace simbo {
	class Context;
	class HostFactory;

	/**
	\brief Laptop. 

	Klasa modeluje laptopa podłączanego do jednej z ustalonego zbioru sieci lokalnych (np. "praca" i "dom").
	*/
	class Laptop : public MobileHost {
	public:
		typedef MarkovSwitcher<local_network_ptr> local_network_switcher_t;

		/** \brief Konstruktor. Wołany z wewnątrz klasy HostFactory.
		\param id Zobacz MobileHost::MobileHost
		\param operating_system Zobacz MobileHost::MobileHost
		\param local_network_switcher Przełącza pomiędzy sieciami lokalnymi.
		\throws Zobacz MobileHost::MobileHost
		*/
		Laptop(id_t id, operating_system_ptr&& operating_system, local_network_switcher_t&& local_network_switcher, Passkey<HostFactory> /*key*/);

		/// Zwróć widok const na wektor sieci lokalnych.
		ConstView<std::vector<local_network_ptr>> get_local_networks_view() const {
			return make_const_view(local_network_switcher_.get_values());
		}		

		HostType get_type() const override {
			return HostType::LAPTOP;
		}
	private:
		local_network_ptr update_local_network(Context& ctx) override;

		/// Przełącza pomiędzy sieciami lokalnymi.
		local_network_switcher_t local_network_switcher_;
	};
}
