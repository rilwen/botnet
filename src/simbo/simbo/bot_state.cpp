#include <ostream>
#include <string>
#include "bot_state.hpp"
#include "exceptions.hpp"
#include "json.hpp"

namespace simbo {
	

	template <> void to_json(json& j, BotState bot_state) {
		if (bot_state != BotState::NONE) {
			j = to_string(bot_state);
		} else {
			j = nullptr;
		}
	}

	template <> void from_json(const json& j, BotState& bot_state) {
		if (!j.empty()) {
			try {
				from_string(j.get<std::string>(), bot_state);
			} catch (std::exception& e) {
				throw DeserialisationError("BotState", j.dump(), e);
			}
		} else {
			bot_state = BotState::NONE;
		}
		
	}
}