#include "exceptions.hpp"
#include "id.hpp"

namespace simbo {
	namespace Id {
		id_t get_next_good_id(id_t& next_id) {
			if (next_id == undefined_id()) {
				throw std::domain_error(boost::locale::translate("Next ID is undefined (ran out of valid IDs?)"));
			}
			if (next_id < min_good_id()) {
				throw std::domain_error(boost::locale::translate("Next ID is too low"));
			}
			auto id = next_id;
			if (id < max_good_id()) {
				++next_id;
			} else {
				// Zaznacz że numery ID się wyczerpały.
				next_id = undefined_id();
			}
			return id;
		}
	}	
}