#pragma once
#include <memory>
#include <vector>
#include "exceptions.hpp"
#include "markov_process.hpp"
#include "tracked_value.hpp"

namespace simbo {

	/** Przełącza się pomiędzy różnymi wartościami zgodnie z procesem Markowa.

	\tparam V Typ wartości procesu.
	*/
	template <class V> class MarkovSwitcher {
	public:
		typedef V value_t;
		typedef std::vector<V> values_vec;
		typedef std::shared_ptr<MarkovProcess<int>> switching_process_ptr;

		/** \brief Konstruktor.
		\param switching_process Proces zwracający indeks nowej wartości.
		\param values Wektor możliwych wartości. Wartości mogą się powtarzać.
		\param initial_value Wartość początkowa (jeden z elementów \p values), przed pierwszym wywołaniem funkcji #update().
		\throw std::invalid_argument Jeżeli \p switching_process jest null albo \p values jest pusty. Jeżeli \p initial_value nie jest elementem \p values.
		*/
		MarkovSwitcher(switching_process_ptr switching_process, values_vec&& values, V initial_value)
			: switching_process_(switching_process),
			value_(initial_value) {
			if (!switching_process) {
				throw std::invalid_argument(boost::locale::translate("Switching process is null"));
			}
			if (values.empty()) {
				throw std::invalid_argument(boost::locale::translate("Empty values vector"));
			}
			const auto it = std::find(values.begin(), values.end(), initial_value);
			if (it == values.end()) {
				throw std::invalid_argument(boost::locale::translate("Initial value not present in values vector"));
			}
			current_index_ = TrackedValue<int>(static_cast<int>(std::distance(values.begin(), it)), -infinite_time());
			values_ = std::move(values);
			number_values_ = static_cast<int>(values_.size());
		}

		/// Konstruktor przenoszący.
		MarkovSwitcher(MarkovSwitcher<V>&&) = default;

		MarkovSwitcher(const MarkovSwitcher<V>&) = delete;
		MarkovSwitcher<V>& operator=(const MarkovSwitcher<V>&) = delete;

		/// Zwróć aktualną wartość.
		V get_current_value() const {
			return value_;
		}

		/// Zwróć czas ostatniej aktualizacji.
		seconds_t get_last_update_time() const {
			return current_index_.get_last_update_time();
		}

		/// Zwróć czas ostatniej zmiany.
		seconds_t get_last_change_time() const {
			return current_index_.get_last_change_time();
		}

		/// Uaktualnij wartość.
		/// \param rng Referencja do generatora liczb losowych.
		/// \param time Czas aktualizacji.
		/// \throws std::domain_error Jeżeli time jest spoza zakresu czasu na którym określony jest proces generujący indeksy wartości, albo jeżeli time <= #get_last_update_time().
		/// \throws std::out_of_range Jeżeli zostanie wygenerowany indeks ujemny albo spoza zakresu wektora wartości.
		/// \return Nowa wartość.
		V update(RNG& rng, seconds_t time) {
			int new_index;
			if (current_index_.get_last_update_time() == - infinite_time()) { // Wybieramy wartość po raz pierwszy.
				new_index = switching_process_->instant_switch(rng, current_index_.get_value(), time);
			} else {
				new_index = switching_process_->get_next_state(rng, current_index_.get_value(), get_last_update_time(), time);
			}
			if (new_index < 0) {
				throw std::out_of_range(boost::locale::translate("New index value is negative"));
			}
			if (new_index >= number_values_) {
				throw std::out_of_range(boost::locale::translate("New index value too large"));
			}
			current_index_.update(new_index, time);
			value_ = values_[new_index];			
			return value_;
		}

		/// Zwróć stałą referencję do wektora możliwych wartości.
		const std::vector<V>& get_values() const {
			return values_;
		}
	private:
		/// Możliwe wartości.
		values_vec values_;

		/// Aktualny index wartości zmiennej.
		TrackedValue<int> current_index_;

		/// Proces generujący indeks nowej wartości.
		switching_process_ptr switching_process_;

		/// Aktualna wartość zmiennej.
		V value_;

		/// Liczba wartości.
		int number_values_;
	};
}
