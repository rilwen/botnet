#pragma once
#include <memory>
#include <unordered_map>
#include <utility>
#include "antivirus.hpp"
#include "hasher.hpp"
#include "json_fwd.hpp"
#include "operating_system.hpp"

namespace simbo {
	template <class> class SparseDiscreteRandomVariable;

	/*! \brief Konfiguracja hostów w sieci. */
	struct HostConfig {

		typedef std::unordered_map<os_spec_t, std::shared_ptr<SparseDiscreteRandomVariable<Antivirus::full_spec_t>>, Hasher> antivirus_cfg_t;

		/// Zmienna losowa generująca rodzaj, numer wersji i numer edycji systemu operacyjnego.
		std::shared_ptr<SparseDiscreteRandomVariable<os_full_spec_t>> operating_system;

		/// Zmienne losowe generujące nazwę i numer edycji antywirusa dla dla danego rodzaju i numeru wersji systemu operacyjnego.
		/// Pusta nazwa oznacza brak antywirusa. Jeżeli brakuje konfiguracji dla danego systemu, oznacza to brak antywirusa.
		antivirus_cfg_t antivirus;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt HostConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję HostConfig#validate().
	void from_json(const json& j, HostConfig& config);
}