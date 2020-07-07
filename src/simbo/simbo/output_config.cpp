#include "data_storage_format.hpp"
#include "exceptions.hpp"
#include "json.hpp"
#include "output_config.hpp"
#include "utils.hpp"

namespace simbo {	
	void OutputConfig::validate() const {
		// Rzuci wyjątkiem jeżeli rozszerzenie jest zabronione.
		if (!sinkhole_data_filename.empty()) {
			data_storage_format_from_extension(get_filename_extension(sinkhole_data_filename));
		}
	}

	void from_json(const json& j, OutputConfig& config) {
		validate_keys(j, "OutputConfig", { }, { "sinkhole_data_filename", "network_structure_filename", "sinkhole_reduced_data_filename"});
		try {
			if (j.count("sinkhole_data_filename")) {
				config.sinkhole_data_filename = j["sinkhole_data_filename"].get<std::string>();
			}
			if (j.count("network_structure_filename")) {
				config.network_structure_filename = j["network_structure_filename"].get<std::string>();
			}			
			if (j.count("sinkhole_reduced_data_filename")) {
				config.sinkhole_reduced_data_filename = j["sinkhole_reduced_data_filename"].get<std::string>();
			}
			config.validate();
		} catch (std::exception& e) {
			throw DeserialisationError("OutputConfig", j.dump(), e);
		}
	}
}
