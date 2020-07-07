#pragma once
#include <memory>
#include <utility>
#include "date_time.hpp"
#include "operating_system_type.hpp"
#include "software.hpp"

namespace simbo {

	/**
	\brief Abstrakcyjna klasa reprezentująca system operacyjny.

	Klasa OperatingSystem dostarcza informacji o tym kiedy łaty na dziury bezpieczeństwa są dostępne dla sysadminów.
	Obiekty OperatingSystem są dzielone pomiędzy hostami.
	*/
	class OperatingSystem: public Software<std::pair<OperatingSystemType, int>> {
	public:
		/** \brief Konstruktor.
		\param spec Specyfikacja (typ i numer wersji).
		\param vendor Dostawca systemu.
		\param release Numer edycji systemu.
		\throw std::invalid_argument Jeżeli vendor == nullptr.
		*/
		OperatingSystem(spec_t spec, std::shared_ptr<vendor_t> vendor, int release);

		/// Zwróć rodzaj systemu.
		OperatingSystemType get_type() const {
			return get_spec().first;
		}

		/// Zwróć numer wersji systemu.
		int get_version_number() const {
			return get_spec().second;
		}

		/// Czy system aktualizuje się w trakcie wyłączania.
		virtual bool update_at_shutdown() const = 0;

		/** Metoda fabryczna.
		\param vendor Dostawca systemu.
		\param os_spec Specyfikacja.
		\param release Numer edycji.
		\return Wskaźnik do implementacji.
		\throw std::invalid_argument Jeżeli vendor == nullptr. Jeżeli typ systemu nie jest rozpoznany.
		*/
		static std::unique_ptr<OperatingSystem> build(std::shared_ptr<vendor_t> vendor, spec_t os_spec, int release);

		/// Czy specyfikacja oznacza brak systemu operacyjnego.
		static bool is_null(spec_t spec) {
			return false;
		}
	};

	typedef OperatingSystem::spec_t os_spec_t;
	typedef OperatingSystem::full_spec_t os_full_spec_t;
}
