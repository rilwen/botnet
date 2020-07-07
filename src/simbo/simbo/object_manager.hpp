#pragma once
#include <memory>
#include <vector>
#include "exceptions.hpp"
#include "managed_object.hpp"
#include "passkey.hpp"

namespace simbo {
	/** \brief Klasa zarządzająca obiektami: szablon bazowy.
	\tparam O Klasa zarządzana (dziedzicząca z ManagedObject<O, M>).
	\tparam M Klasa zarządzające (dziedzicząca z ObjectManager<O, M>).
	*/
	template <class O, class M> class ObjectManager {
	public:
		// Skrócone nazwy typów.
		typedef std::unique_ptr<O> object_ptr;
		typedef std::vector<object_ptr> objects_vec;

		/// Domyślny konstruktor.
		ObjectManager() {}

		/// Konstruktor przenoszący.
		ObjectManager(ObjectManager&& other) = default;

		/**
		\brief Dodaj nowy obiekt.
		\param manager Referencja do obiektu klasy dziedziczącej (konkretnego managera).
		\param object Wskaźnik do zarządzanego obiektu.
		Funkcja przejmuje obiekt na własność i ustawia managera obiektu.
		\throws std::invalid_argument Jeżeli object == nullptr.
		*/
		void add_object(const M& manager, object_ptr&& object, Passkey<M>) {
			if (!object) {
				throw std::invalid_argument(boost::locale::translate("Object is null"));
			}
			object->set_manager(manager, Passkey<ObjectManager<O, M>>());
			objects_.push_back(std::move(object));
		}

		/// Zwróć referencję do wektora zarządzanych obiektów.
		const objects_vec& get_objects() const {
			return objects_;
		}
	private:
		/// Zarządzane obiekty.
		objects_vec objects_;
	};
}
