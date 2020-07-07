#pragma once
#include <memory>
#include <vector>
#include "local_network.hpp"
#include "object_manager.hpp"

namespace simbo {
	/** \brief Modeluje administratora sieci.
	*/
	class NetworkAdmin : private ObjectManager<LocalNetwork, NetworkAdmin> {
	public:
		// Skrócone nazwy typów.
		typedef object_ptr network_ptr;
		typedef objects_vec networks_vec;

		NetworkAdmin() {}

		/// Konstruktor przenoszący.
		NetworkAdmin(NetworkAdmin&& other) = default;

		virtual ~NetworkAdmin() {}

		/**
		\brief Dodaj nową sieć.

		Funkcja przejmuje sieć lokalną na własność i ustawia this jako administratora sieci.

		\param network Wskaźnik do dodawanej sieci.

		\throws std::invalid_argument Jeżeli network == nullptr.
		*/
		void add_network(network_ptr&& network) {
			add_object(*this, std::move(network), Passkey<NetworkAdmin>());
		}

		/// Zwróć referencję do wektora zarządzanych sieci lokalnych.
		const networks_vec& get_networks() const {
			return get_objects();
		}
	};
}
