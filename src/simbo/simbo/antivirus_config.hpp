#pragma once
#include "date_time.hpp"
#include "json_fwd.hpp"

namespace simbo {
	/// Konfiguracja antywirusa.
	struct AntivirusConfig {
		/// Prawdopodobieństwo wykrycia próby infekcji (niezależnie od typu botnetu).
		double infection_attempt_detection_probability;

		/// Czy antywirus ściąga uaktualnienia (nowe edycje) podczas startu hosta.
		bool update_at_startup;

		/// Czy antywirus skanuje system po starcie hosta.
		bool scan_at_startup;

		/// Czy antywirus skanuje system po uaktualnieniu edycji.
		bool scan_after_update;

		/// Planowany odstęp czasowy pomiędzy skanami.
		TimeDuration scan_interval;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		void validate() const;
	};

	/// Deserializuj AntivirusConfig z formatu JSON.
	void from_json(const json& j, AntivirusConfig& config);
}
