#pragma once
#include <cassert>
#include "exceptions.hpp"
#include "id.hpp"

namespace simbo {
	/// Fabryka numerów ID.
	class IdFactory {
	public:
		/// Skrócone nazwy typów.
		typedef Id::id_t id_t;

		/// Domyślny konstruktor.
		IdFactory()
			: next_id_(Id::min_good_id()) {}

		/// Zwróć następne dobre ID.
		/// \throw std::runtime_error Jeżeli zakres numerów ID się wyczerpał.
		id_t get_next_good_id() {
			assert(next_id_ >= Id::min_good_id());
			try {
				return Id::get_next_good_id(next_id_);
			} catch (std::domain_error&) {
				throw std::runtime_error(boost::locale::translate("Ran out of ID numbers"));
			}
		}
	private:
		id_t next_id_;
	};
}