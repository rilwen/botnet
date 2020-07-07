#pragma once
#include <typeinfo>
#include "exceptions.hpp"
#include "own_time_counter.hpp"
#include "tracked_value.hpp"

namespace simbo {

	/** Wersja TrackedValue która śledzi również ile upłynęło czasu "własnego" (np. czasu kiedy Host jest włączony) od ostatniej zmiany albo odświeżenia.
	\tparam V Typ wartości.
	*/
	template <class V> class TrackedValueOwnTime : public TrackedValue<V> {
	public:
		/** \brief Konstruktor.
		\param value Zob. TrackedValue<V>::TrackedValue
		\param time Zob. TrackedValue<V>::TrackedValue
		\throw std::domain_error Zob. TrackedValue<V>::TrackedValue.
		*/
		TrackedValueOwnTime(V value, seconds_t time)
			: TrackedValue<V>(value, time),
			own_time_since_last_update_(time),
			own_time_since_last_change_(time) {
		}

		/// Domyślny konstruktor kopiujący.
		TrackedValueOwnTime(const TrackedValueOwnTime<V>&) = default;

		/// Domyślny konstruktor przenoszący.
		TrackedValueOwnTime(TrackedValueOwnTime<V>&&) = default;

		/// Domyślny operator przypisania.
		TrackedValueOwnTime<V>& operator=(const TrackedValueOwnTime<V>&) = default;

		/// Operator "jest równy".
		bool operator==(const TrackedValueOwnTime<V>& other) const {
			if (this == &other) {
				return true;
			}
			return TrackedValue<V>::operator==(other) && own_time_since_last_change_ == other.own_time_since_last_change_ && own_time_since_last_update_ == other.own_time_since_last_update_;
		}

		/// Operator "nie jest równy".
		bool operator!=(const TrackedValueOwnTime<V>& other) const {
			return !(*this == other);
		}

		/** \brief Resetuje akumulowany czas własny na zero.
		Funkcja obsługuje również uaktualnianie czasu własnego, resetując odpowiednie liczniki.
		\param new_value Zob. TrackedValue#update
		\param new_time Zob. TrackedValue#update
		\param own_time_was_running Czy czas własny płynął w okresie pomiędzy ostatnim uaktualnieniem czasu własnego i new_time.
		\see TrackedValue#update.
		*/
		TrackedValueOwnTime<V>& update(const V new_value, const seconds_t new_time, bool own_time_was_running) {
			TrackedValue<V>::update(new_value, new_time);
			assert(own_time_since_last_update_.get_last_update_wall_time() == own_time_since_last_change_.get_last_update_wall_time());
			if (new_time > own_time_since_last_update_.get_last_update_wall_time()) {
				update_own_time(new_time, own_time_was_running);
			}
			own_time_since_last_update_.reset(new_time);
			if (get_last_change_time() == new_time) {
				own_time_since_last_change_.reset(new_time);
			}
			return *this;
		}

		/// Zwróć czas własny jaki upłynął od ostatniej aktualizacji.
		seconds_t get_own_time_since_last_update() const {
			return own_time_since_last_update_.get_own_time();
		}

		/// Zwróć czas własny jaki upłynął od ostatniej zmiany wartości.
		seconds_t get_own_time_since_last_change() const {
			return own_time_since_last_change_.get_own_time();
		}

		/*! Uaktualnij czas własny.
		\param own_time_was_running Czy czas własny płynął w okresie pomiędzy ostatnim uaktualnieniem czasu własnego i new_time.
		\throw std::domain_error Jeżeli wall_time jest w przeszłości. Zob. OwnTimeCounter#update().
		*/
		void update_own_time(const seconds_t wall_time, bool own_time_was_running) {
			try {
				own_time_since_last_update_.update(wall_time, own_time_was_running);
				own_time_since_last_change_.update(wall_time, own_time_was_running);
			} catch (const std::domain_error& e) {
				throw std::domain_error((boost::locale::format("TrackedValueOwnTime<{1}>: {2}") % typeid(V).name() % e.what()).str());
			}
		}
	private: 
		OwnTimeCounter own_time_since_last_update_;
		OwnTimeCounter own_time_since_last_change_;
	};
}
