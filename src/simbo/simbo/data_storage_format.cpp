#include "data_storage_format.hpp"
#include "exceptions.hpp"

namespace simbo {
	DataStorageFormat data_storage_format_from_extension(const std::string& extension) {
		if (extension == "csv") {
			return DataStorageFormat::CSV;
		} else if (extension == "bin") {
			return DataStorageFormat::BIN;
		} else if (extension == "") {
			return DataStorageFormat::NONE;
		} else {
			throw std::invalid_argument((boost::locale::format("Unsupported extension: \"{1}\"") % extension).str());
		}
	}
}