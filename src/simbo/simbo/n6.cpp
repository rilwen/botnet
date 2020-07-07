#include <ostream>
#include "n6.hpp"

namespace simbo {
	namespace n6 {
		const DateTime EPOCH(Date(1970, 1, 1), TimeDuration(0, 0, 0));

		Header::Header(size_t n_number_rows)
			: number_rows(static_cast<uint64_t>(n_number_rows)) {
			std::fill(reserved, reserved + reserved_size, 0);
		}

		void write_header(size_t number_rows, std::ostream& out) {
			Header header(number_rows);
			out.write((char*)&header, sizeof(Header));
		}
	}
}