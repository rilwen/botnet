#pragma once
#include <memory>
#include "json_fwd.hpp"

namespace simbo {

	class PausingJumpMarkovProcess;
	class Schedule;
	template <class> class SparseDiscreteRandomVariable;

	/** Konfiguracja administratora(ki) systemu.
	*/
	struct SysadminConfig {
		typedef std::shared_ptr<PausingJumpMarkovProcess> maintenance_trigger_ptr;
		
		/// Proces który skacze kiedy administrator ma się zająć swoimi komputerami, losowo przydzielany każdemu sysadminowi opisanemu tą konfiguracją.
		std::shared_ptr<SparseDiscreteRandomVariable<maintenance_trigger_ptr>> maintenance_triggers;

		/** \brief Sprawdź poprawność konfiguracji.
		\throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
		\throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
		*/
		void validate() const;
	};

	/** \brief Odczytaj SysadminConfig z formatu JSON.
	\param j Dane JSON. Wymagane pola: "maintenance_triggers".
	\param schedule Harmonogram.
	\param sysadmin_config Referencja do tworzonej konfiguracji.
	\throw DeserialisationError W razie problemów z deserializacją.
	*/
	void from_json(const json& j, const Schedule& schedule, SysadminConfig& sysadmin_config);
}