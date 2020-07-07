#pragma once
#include "enums.hpp"
#include "enums_json.hpp"
#include "json_fwd.hpp"

namespace simbo {
	/// Stan bota. Kolejne wartości mają odpowiadać kolejnym liczbom całkowitym.
	enum class BotState {

		/// Bot konfiguruje się po infekcji: ściąga dodatkowy kod, rejestruje się w botnecie peer-to-peer, itd.
		/// Ta wartość musi być pierwsza.
		SETTING_UP,

		/// Bot pracuje: fedruje Bitcoiny, rozsyła spam, wrzuca polityczne komentarze w mediach społecznościowych, itd.
		WORKING,

		/// Bot propaguje infekcję na inne komputery.
		PROPAGATING,

		/// Bot odpoczywa.
		DORMANT,

		/// Host nie jest botem.
		/// Ta wartość musi być ostatnia.
		NONE
	};

	template <> struct EnumNames<BotState> {
		static constexpr std::array<const char*, 5> NAMES = {
			"SETTING_UP",
			"WORKING",
			"PROPAGATING",
			"DORMANT",
			"NONE"
		};
	};

	/// Specjalizacja funkcji to_json z enums.hpp.
	/// Konwertuje BotState::NONE na pusty element.
	template <> void to_json(json& j, BotState bot_state);

	/// Specjalizacja funkcji from_json z enums.hpp.
	/// Konwertuje pusty element na BotState::NONE.
	template <> void from_json(const json& j, BotState& bot_state);
}
