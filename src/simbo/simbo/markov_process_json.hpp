#pragma once
#include <memory>
#include "json.hpp"
#include "jump_markov_process_json.hpp"
#include "markov_process.hpp"
#include "pausing_jump_markov_process.hpp"
#include "schedule.hpp"
#include "semi_predetermined_process.hpp"

namespace simbo {
	/** \brief Deserializuj proces Markova z formatu JSON, dopasowując go do podanego harmonogramu.
	Wymagane pola: "type", "params". Wymagane dla pewnych wartości pola "type": "subtype".
	\param j Dane JSON
	\param schedule Harmonogram
	\param process Wskaźnik do procesu Markowa. Jeżeli wywołanie funkcji zakończy się sukcesem, to wskaźnik nie jest null.
	\throw DeserialisationError Jeżeli pole "type" ma niespodziewaną wartość, albo brakuje któregoś z wymaganych pól.
	*/
	template <class T> void from_json(const json& j, const Schedule& schedule, std::unique_ptr<MarkovProcess<T>>& process) {
		validate_keys(j, "MarkovProcess", { "type", "params" }, { "subtype" });
		const std::string mp_type = j["type"];
		const json& j_params = j["params"];
		if (mp_type == "semi_predetermined") {
			expect_key(j, "MarkovProcess", "subtype");
			const std::string mp_subtype = j["subtype"];
			std::unique_ptr<SemiPredeterminedProcess<T>> derived_ptr;
			if (mp_subtype == "seasonal_weekly") {
				from_json_seasonal_weekly(j_params, schedule, derived_ptr);
			} else {
				throw DeserialisationError("MarkovProcess", j.dump(), (boost::locale::format("Unknown subtype \"{1}\" for type \"{2}\"") % mp_subtype % mp_type).str());
			}
			process = std::move(derived_ptr);
		} else if (is_jump_markov_process_type(mp_type)) {
			deserialise_derived<JumpMarkovProcess>(j, schedule, process, "MarkovProcess", mp_type);
		} else if (mp_type == "pausing_jump") {
			deserialise_derived<PausingJumpMarkovProcess>(j, schedule, process, "MarkovProcess", mp_type);
		} else {
			throw DeserialisationError("MarkovProcess", j.dump(), (boost::locale::format("Unknown type: {1}") % mp_type).str());
		}
	}
}
