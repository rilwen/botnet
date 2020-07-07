#pragma once
#include "date_time.hpp"
#include "json_fwd.hpp"

namespace simbo {
	/// Opisuje jak często powinna być wykonywana dana akcja.
	class ActionInterval {
	public:
		/** \brief Konstruktor.

		\param period Odstęp czasowy, > 0.
		\param by_wall_clock Czy czas pomiędzy wykonaniami akcji biegnie zawsze (true) czy tylko wtedy, kiedy agent je wykonujący (np. Host) jest aktywny.

		\throw std::domain_error Jeżeli period nie jest dodatni.
		*/
		ActionInterval(seconds_t period, bool by_wall_clock);

		/// Konstruktor domyślny, z odstępem 1s i by_wall_clock == true.
		ActionInterval()
			: ActionInterval(1, true) {}

		ActionInterval(const ActionInterval&) = default;
		ActionInterval(ActionInterval&&) = default;
		ActionInterval& operator=(const ActionInterval&) = default;

		seconds_t get_period() const {
			return period_;
		}

		bool is_by_wall_clock() const {
			return by_wall_clock_;
		}
	private:
		seconds_t period_;

		bool by_wall_clock_;
	};

	/// Odczytaj obiekt ActionInterval z formatu JSON.
	/// Wymagane pola: period, by_wall_clock.
	/// \param j Dane JSON.
	/// \param action_interval Konfigurowany obiekt.
	/// \throw std::domain_error Jeżeli period nie jest dodatni.
	void from_json(const json& j, ActionInterval& action_interval);
}

//namespace nlohmann {
//	template <>
//	struct adl_serializer<simbo::ActionInterval> {
//		/// \brief Odczytaj obiekt ActionInterval z formatu JSON.
//		/// Wymagane pola: period, by_wall_clock.
//		static simbo::ActionInterval from_json(const json& j);
//
//		static void to_json(json& j, const simbo::ActionInterval& ai);
//	};
//}
