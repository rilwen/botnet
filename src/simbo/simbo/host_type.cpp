#include "host_type.hpp"

namespace simbo {
	bool is_host_type_fixed(HostType host_type) {
		return host_type != HostType::LAPTOP;
	}
}