#pragma once
#include <cassert>
#include "passkey.hpp"

namespace simbo {

	template <class O, class M> class ObjectManager;

	/** \brief Obiekt którego cykl życia jest zarządzany przez inny obiekt (ObjectManager).
	
	Szablon umożliwia zarządzanie obiektami jednej klasy przed drugą. Przechowuje wskaźnik do zarządcy,
	ale udostępnia jedynie referencję - zakładamy, że ManagedObject będzie zawsze rejestrowany u zarządcy.

	\tparam O Klasa zarządzana (dziedzicząca z ManagedObject<O, M>).
	\tparam M Klasa zarządzające (dziedzicząca z ObjectManager<O, M>).
	*/
	template <class O, class M> class ManagedObject {
	public:
		typedef M manager_t;

		/// Zainicjalizuj obiekt bez zarządcy.
		ManagedObject()
			: manager_(nullptr) {

		}

		ManagedObject(const ManagedObject<O, M>&) = delete;
		ManagedObject<O, M>& operator=(const ManagedObject<O, M>&) = delete;

		/// Ustaw zarządcę. Wołane przez funkcję klasy ObjectManager<O, M>.
		void set_manager(const manager_t& manager, Passkey<ObjectManager<O, M>>) {
			assert(!manager_);
			manager_ = &manager;
		}

		/// Zwróć wskaźnik do zarządcy obiektu.
		const manager_t& get_manager() const {
			assert(manager_);
			return *manager_;
		}
	private:
		/// Wskaźnik do zarządcy obiektu.
		const manager_t* manager_;
	};
}
