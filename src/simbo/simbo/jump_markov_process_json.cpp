#include "json.hpp"
#include "jump_markov_process_json.hpp"
#include "poisson_process.hpp"
#include "schedule.hpp"
#include "simple_poisson_process.hpp"

namespace simbo {
	bool is_jump_markov_process_type(const std::string& mp_type) {
		return mp_type == "poisson";
	}

	void from_json(const json& j, const Schedule& schedule, std::unique_ptr<JumpMarkovProcess>& process) {
		validate_keys(j, "JumpMarkovProcess", { "type", "params" }, { "subtype" });
		try {
			const std::string mp_type = j["type"];
			const json& j_params = j["params"];
			if (mp_type == "poisson") {
				expect_key(j, "JumpMarkovProcess", "subtype");
				const std::string mp_subtype = j["subtype"];
				if (mp_subtype == "simple") {
					std::unique_ptr<SimplePoissonProcess> derived_ptr;
					from_json(j_params, derived_ptr);
					process = std::move(derived_ptr);
				} else if (mp_subtype == "seasonal_weekly") {
					std::unique_ptr<PoissonProcess> derived_ptr;
					from_json_seasonal_weekly(j_params, schedule, derived_ptr);
					process = std::move(derived_ptr);
				} else {
					throw std::runtime_error((boost::locale::format("Unknown subtype \"{1}\" for type \"{2}\"") % mp_subtype % mp_type).str());
				}
			} else {
				throw std::runtime_error((boost::locale::format("Unknown type: {1}") % mp_type).str());
			}
		} catch (const std::exception& e) {
			throw DeserialisationError("JumpMarkovProcess", j.dump(), e);
		}
	}
}
