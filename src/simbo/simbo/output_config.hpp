#pragma once
#include <string>
#include "json_fwd.hpp"

namespace simbo {
	/// Konfiguracja procesu generowania wyników.
	struct OutputConfig {
		/// Nazwa pliku na dane sinkhole'a. Format zapisu zależy od rozszerzenia pliku: HDF5 dla .h5, CSV dla .csv.
		/// Inne rozszerzenia nie są dozwolone. Opcjonalne.
		std::string sinkhole_data_filename;

		/// Nazwa pliku do którego zostanie zapisana struktura wygenerowanej sieci w formacie CSV. Opcjonalne.
		std::string network_structure_filename;

		/// Nazwa pliku na zredukowane dane sinkhole'a. Dane zostaną zapisane w formacie CSV. Opcjonalne.
		std::string sinkhole_reduced_data_filename;

		/// Sprawdź konfigurację.
		/// \throw std::invalid_argument Jeżeli sinkhole_data_filename ma niedozwolone rozszerzenie.
		void validate() const;
	};

	/// Odczytaj obiekt OutputConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję OutputConfig#validate().
	void from_json(const json& j, OutputConfig& config);
}
