#include "action_interval.hpp"
#include "date_time.hpp"
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {
	ActionInterval::ActionInterval(seconds_t period, bool by_wall_clock)
		: period_(period), by_wall_clock_(by_wall_clock) {
		if (!(period_ > 0)) {
			throw std::domain_error(boost::locale::translate("Period must be positive"));
		}
	}

	void from_json(const json& j, ActionInterval& action_interval) {
		validate_keys(j, "ActionInterval", { "period", "by_wall_clock" }, {});
		try {
			const simbo::seconds_t period = j["period"];
			const bool by_wall_clock = j["by_wall_clock"];
			action_interval = ActionInterval(period, by_wall_clock);
		} catch (std::exception& e) {
			throw DeserialisationError("ActionInterval", j.dump(), e);
		}
	}
}

//namespace nlohmann {
//	simbo::ActionInterval adl_serializer<simbo::ActionInterval>::from_json(const json& j) {
//		simbo::validate_keys(j, "ActionInterval", { "period", "by_wall_clock" }, {});
//		
//	}
//
//	void adl_serializer<simbo::ActionInterval>::to_json(json& j, const simbo::ActionInterval& ai) {
//		j = {
//			{"period", ai.get_period()},
//			{"by_wall_clock", ai.is_by_wall_clock()}
//		};
//	}
//}
//
