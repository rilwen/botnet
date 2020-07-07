#pragma once
#include <memory>
#include "json_fwd.hpp"
#include "jump_markov_process.hpp"

namespace simbo {
	class Schedule;

	/// Sprawdź czy typ procesu Markowa jest skaczący.
	/// \param mp_type Typ procesu Markowa w danych JSON.
	bool is_jump_markov_process_type(const std::string& mp_type);

	/** \brief Deserializuj skaczący proces Markowa z formatu JSON, dopasowując go do podanego harmonogramu.
	Wymagane pola: "type", "params". Wymagane dla pewnych wartości pola "type": "subtype".
	\param j Dane JSON
	\param schedule Harmonogram
	\param process Wskaźnik do procesu Markowa. Jeżeli wywołanie funkcji zakończy się sukcesem, to wskaźnik nie jest null.
	\throw DeserialisationError Jeżeli pole "type" ma niespodziewaną wartość, albo brakuje któregoś z wymaganych pól.
	*/
	void from_json(const json& j, const Schedule& schedule, std::unique_ptr<JumpMarkovProcess>& process);
}
