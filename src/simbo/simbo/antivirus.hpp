#pragma once
#include <string>
#include "date_time.hpp"
#include "software.hpp"

namespace simbo {

	struct AntivirusConfig;
	class Context;
	class Host;

	/// Program antywirusowy.
	/// Skanuje okresowo hosta, wykrywając i usuwając infekcje.
	class Antivirus : public Software<std::string> {
	public:
		/** \brief Konstruktor.
		\param name Nazwa antywirusa.
		\param vendor Dostawca oprogramowania.
		\param release Wersja edycji.
		\param config Konfiguracja.
		\throw std::invalid_argument Jeżeli vendor == nullptr albo name.empty() albo konfiguracja jest niewłaściwa.
		\throw std::domain_error Jeżeli konfiguracja jest niewłaściwa.
		*/
		Antivirus(const std::string& name, std::shared_ptr<vendor_t> vendor, int release, const AntivirusConfig& config);

		/// Zwróć prawdopodobieństwo wykrycia próby infekcji (niezależnie od typu botnetu).
		double get_infection_attempt_detection_probability() const {
			return infection_attempt_detection_probability_;
		}

		/// Czy antywirus ściąga uaktualnienia (nowe edycje) w trakcie startu hosta.
		bool get_update_at_startup() const {
			return update_at_startup_;
		}

		/// Czy antywirus skanuje system po starcie hosta.
		bool get_scan_at_startup() const {
			return scan_at_startup_;
		}

		/// Czy antywirus skanuje system po uaktualnieniu.
		bool get_scan_after_update() const {
			return scan_after_update_;
		}

		/// Zwróć zamierzony odstęp pomiędzy skanami (w sekundach).
		seconds_t get_scan_interval() const {
			return scan_interval_;
		}

		/// Czy specyfikacja oznacza brak antywirusa.
		static bool is_null(spec_t spec) {
			return spec.empty();
		}
	private:
		std::string name_;
		double infection_attempt_detection_probability_;
		bool update_at_startup_;
		bool scan_at_startup_;
		bool scan_after_update_;
		seconds_t scan_interval_;
	};
}