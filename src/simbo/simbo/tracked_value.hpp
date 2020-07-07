#pragma once
#include <boost/locale.hpp>
#include "date_time.hpp"

namespace simbo {

	/**
	\brief Wartość śledzona w czasie.
	Przechowywane są: wartość, czas ostatniej aktualizacji, czas ostatniej zmiany. Czasy nie mogą się zmniejszać.
	\tparam V Typ wartości.
	*/
	template <class V> class TrackedValue {
	public:
		/** \brief Konstruktor.
		\param value Wartość.
		\param time Czas.
		\throw std::domain_error Jeżeli is_time_undefined(time) == true.
		*/
		TrackedValue(V value, seconds_t time)
			: value_(value),
			last_update_time_(time),
			last_change_time_(time) {
			if (is_time_undefined(time)) {
				throw std::domain_error(boost::locale::translate("Undefined time"));
			}
		}

		/// Domyślny konstruktor, ustawia wartość na V() i czas na nieokreślony.
		TrackedValue()
			: value_(V()),
			last_update_time_(undefined_time()),
			last_change_time_(undefined_time()) {
		}

		/// Domyślny konstruktor kopiujący.
		TrackedValue(const TrackedValue<V>&) = default;

		/// Domyślny konstruktor przenoszący.
		TrackedValue(TrackedValue<V>&&) = default;

		/// Domyślny operator przypisania.
		TrackedValue<V>& operator=(const TrackedValue<V>&) = default;

		/// Operator "jest równy".
		bool operator==(const TrackedValue<V>& other) const {
			if (this == &other) {
				return true;
			}
			return value_ == other.value_ && last_change_time_ == other.last_change_time_ && last_update_time_ == other.last_update_time_;
		}

		/// Operator "nie jest równy".
		bool operator!=(const TrackedValue<V>& other) const {
			return !(*this == other);
		}

		/// Zwróć wartość.
		V get_value() const {
			return value_;
		}

		/// Zwróć czas ostatniej aktualizacji.
		/// \returns Wartość większa lub równa wartości zwracanej przez #get_last_change_time(), albo undefined_time().
		seconds_t get_last_update_time() const {
			return last_update_time_;
		}

		/// Zwróć czas ostatniej zmiany.
		/// \returns Wartość mniejsza lub równa wartości zwracanej przez #get_last_update_time(), albo undefined_time().
		seconds_t get_last_change_time() const {
			return last_change_time_;
		}

		/**
		\brief Uaktualnij wartość.
		\param new_value Nowa wartość.
		\param new_time Nowy czas, poźniejszy niż #get_last_update_time().
		\throws std::logic_error Jeżeli #get_last_update_time() jest niezdefiniowany.
		\throws std::invalid_argument Jeżeli new_time jest niezdefiniowany.
		\throws std::domain_error Jeżeli new_time <= #get_last_update_time().
		\return *this
		*/
		TrackedValue<V>& update(const V new_value, const seconds_t new_time) {
			if (is_time_undefined(last_update_time_)) {
				throw std::logic_error(boost::locale::translate("Cannot update value with undefined old timestamp"));
			}
			if (is_time_undefined(new_time)) {
				throw std::invalid_argument(boost::locale::translate("Cannot update value with undefined new timestamp"));
			}
			if (new_time <= get_last_update_time()) {
				throw std::domain_error(boost::locale::translate("Value is not newer than previous one"));
			}
			if (value_ != new_value) {
				last_change_time_ = new_time;
			}
			value_ = new_value;
			last_update_time_ = new_time;
			return *this;
		}

	private:
		V value_;
		seconds_t last_update_time_;
		seconds_t last_change_time_;
	};
}
