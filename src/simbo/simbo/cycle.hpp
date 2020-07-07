#pragma once
#include <vector>
#include <boost/locale.hpp>
#include "exceptions.hpp"

namespace simbo {
	/**
	\brief Cykl danych.

	Struktura danych pozwalająca na:
	- iterowanie bez końca po liście danych w kolejności indeksów 0, 1, ..., N - 1, 0, 1, ...
	- dodawanie kolejnych wartości do końca listy
	*/
	template <class T> class Cycle{
	public:
		/// Konstruktor domyślny.
		Cycle()
			: next_idx_(0) {

		}


		/// Dodaj nową wartość na końcu.
		void add(const T& value) {
			values_.push_back(value);
		}

		/// Dodaj nową wartość na końcu.
		void add(T&& value) {
			values_.push_back(std::move(value));
		}

		/** 
		\brief Zwróć kolejną wartość.
		\throws std::logic_error Jeżeli cykl jest pusty.
		*/
		const T& next() {
			if (empty()) {
				throw std::logic_error(boost::locale::translate("Cycle is empty"));
			}
			const T& next_value = values_[next_idx_];
			++next_idx_;
			if (static_cast<size_t>(next_idx_) == values_.size()) {
				next_idx_ = 0;
			}
			return next_value;
		}

		/// Sprawdź czy cykl jest pusty.
		bool empty() const {
			return values_.empty();
		}

		/// Przesuń wskaźnik następnej wartości do początku.
		void reset() {
			next_idx_ = 0;
		}

		/// Usuń wszystkie elementy z cyklu.
		void clear() {
			values_.clear();
			reset();
		}
	private:
		/// Wartości.
		std::vector<T> values_;

		/// Wskaźnik do następnej zwracanej wartości.
		int next_idx_;
	};
}
