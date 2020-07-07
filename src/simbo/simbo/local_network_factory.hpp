#pragma once
#include "id_factory.hpp"
#include "local_network_type.hpp"

namespace simbo {
	class LocalNetwork;
	class NetworkAdmin;

	/// Fabryka obiektów klasy LocalNetwork.
	class LocalNetworkFactory {
	public:
		typedef IdFactory::id_t id_t;

		/** Stwórz sieć lokalną i przekaż ją administratorowi sieci.
		\param network_admin Referencja do administratora sieci.
		\param typ Typ sieci lokalnej.
		\param country Kod kraju.
		\return Wskaźnik do sieci.
		*/
		LocalNetwork* make_local_network(NetworkAdmin& network_admin, LocalNetworkType typ, int country);
	private:
		/// Fabryka numerów ID.
		IdFactory id_factory_;
	};
}
