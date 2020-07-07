#pragma once
#include <boost/locale.hpp>
#include "date_time.hpp"

namespace simbo {

	/**
	\brief Wartość stowarzyszona z chwilą czasu. Przechowywany czas nie może się zmniejszyć.
	\tparam V Typ wartości.
	*/
	template <class V> class TimestampedValue {
	public:
		/** \brief Konstruktor.
		\param value Wartość.
		\param time Czas.
		*/
		TimestampedValue(V value, seconds_t time)
			: value_(value),
			time_(time) {
		}

		/// Domyślny konstruktor, ustawia wartość na V() i czas na nieokreślony.
		TimestampedValue()
			: TimestampedValue<V>(V(), undefined_time()) {
		}

		/// Domyślny konstruktor kopiujący.
		TimestampedValue(const TimestampedValue<V>&) = default;

		/// Domyślny konstruktor przenoszący.
		TimestampedValue(TimestampedValue<V>&&) = default;

		/// Domyślny operator przypisania.
		TimestampedValue<V>& operator=(const TimestampedValue<V>&) = default;

		/// Zwróć wartość.
		V get_value() const {
			return value_;
		}

		/// Zwróć czas.
		seconds_t get_time() const {
			return time_;
		}

		/**
		\brief Uaktualnij wartość w późniejszej chwili czasu, jeżeli różni się od aktualnej.
		\param new_value Nowa wartość.
		\param new_time Nowy czas, poźniejszy niż #get_time().
		\throws std::logic_error Jeżeli #get_time() jest niezdefiniowany.
		\throws std::invalid_argument Jeżeli new_time jest niezdefiniowany.
		\throws std::domain_error Jeżeli new_time <= #get_time().
		\return *this
		*/
		TimestampedValue<V>& update_if_different(const V new_value, const seconds_t new_time) {
			if (is_time_undefined(time_)) {
				throw std::logic_error(boost::locale::translate("Cannot update value with undefined old timestamp"));
			}
			if (is_time_undefined(new_time)) {
				throw std::invalid_argument(boost::locale::translate("Cannot update value with undefined new timestamp"));
			}
			if (new_time <= time_) {
				throw std::domain_error(boost::locale::translate("Value is not newer than current one"));
			}
			if (value_ != new_value) {
				value_ = new_value;
				time_ = new_time;
			}
			return *this;
		}

		/**
		\brief Uaktualnij wartość w późniejszej chwili czasu, nawet jeżeli nowa wartość jest ta sama.
		\param new_value Nowa wartość.
		\param new_time Nowy czas, poźniejszy niż #get_time().
		\throws std::logic_error Jeżeli #get_time() jest niezdefiniowany.
		\throws std::invalid_argument Jeżeli new_time jest niezdefiniowany.
		\throws std::domain_error Jeżeli new_time <= #get_time().
		\return *this z ustawioną nową wartością i nowym czasem.
		*/
		TimestampedValue<V>& update(const V new_value, const seconds_t new_time) {
			if (is_time_undefined(time_)) {
				throw std::logic_error(boost::locale::translate("Cannot update value with undefined old timestamp"));
			}
			if (is_time_undefined(new_time)) {
				throw std::invalid_argument(boost::locale::translate("Cannot update value with undefined new timestamp"));
			}
			if (new_time <= time_) {
				throw std::domain_error(boost::locale::translate("Value is not newer than current one"));
			}
			value_ = new_value;
			time_ = new_time;
			return *this;
		}

		/**
		\brief Zmień wartość bez zmieniania czasu.
		\param new_value Nowa wartość.
		\return *this z ustawioną nową wartością i starym czasem.
		*/
		TimestampedValue<V>& reset_value(const V new_value) {
			value_ = new_value;
			return *this;
		}
	private:
		V value_;
		seconds_t time_;
	};

	/// Dodaj stempel czasowy do wartości.
	template <class V> TimestampedValue<V> make_timestamped_value(V value, seconds_t time) {
		return TimestampedValue<V>(value, time);
	}

	template <class T> TimestampedValue<const T *> make_timestamped_const_ptr(TimestampedValue<T *> tv) {
		return TimestampedValue<const T*>(tv.get_value(), tv.get_time());
	}
}
