#pragma once
#include <cstdint>

namespace simbo {
	/// Typy i funkcje związane z numerami ID.
	namespace Id {
		/// Typ przechowujący numery ID.
		typedef int64_t id_t;

		/// Nieokreślony numer ID.
		inline constexpr id_t undefined_id() {
			return 0;
		}

		/// Najmniejszy poprawny numer ID.
		inline constexpr id_t min_good_id() {
			return 1;
		}

		/// Największy poprawny numer ID.
		inline constexpr id_t max_good_id() {
			// Ograniczamy się do zakresu który się mieści w typie int32_t.
			return 2147483647;
		}

		/// Sprawdź czy numer ID jest poprawny.
		inline bool is_good(id_t id) {
			return id > 0;
		}

		/// Sprawdź czy numer ID jest "specjalny". Znaczenie takiego numeru zależy od kontekstu.
		inline bool is_special(id_t id) {
			return id < 0;
		}

		/** \brief Zwróć następny numer ID.

		Jeżeli next_id jest #undefined_id(), funkcja rzuca wyjątek żeby zasygnalizować że zakres numerów się wyczerpał.
		W przeciwnym wypadku next_id jest zwiększane o 1 a jego stara wartość zostanie zwrócona. Jeżeli po zwiększeniu
		o 1 next_id == #max_good_id(), to next_id zostaje ustawione na wartość #undefined_id().

		\param[in,out] next_id Referencja do zmiennej przechowującej następny numer ID. Zmieniana jeżeli funkcja zwraca wynik.
		\throw std::domain_error Jeżeli next_id < #min_good_id().
		\throw std::domain_error Jeżeli next_id == #undefined_id(), co oznacza że zakres numerów ID się wyczerpał.
		\return Następny numer ID.
		*/
		id_t get_next_good_id(id_t& next_id);
	}
}
