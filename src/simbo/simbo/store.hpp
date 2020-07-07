#pragma once
#include <string>
#include <unordered_map>
#include "exceptions.hpp"

namespace simbo {
	/** \brief Przechowalnia obiektów dostępnych przez klucze.
	\tparam T Typ obiektów.
	*/
	template <class T> class Store {
	public:
		/** \brief Dodaj obiekt.
		\param key Klucz.
		\param value Obiekt.
		\throw std::invalid_argument Jeżeli klucz jest pusty albo obiekt o takim kluczu jest już przechowywany.
		*/
		void add(const std::string& key, const T& value);

		/** \brief Sprawdź czy obiekt o danym kluczu jest już przechowywany.
		\param key Klucz.
		\throw std::invalid_argument Jeżeli klucz jest pusty.
		*/
		bool has(const std::string& key) const;

		/** \brief Zwróć referencję do obiektu o danym kluczu.
		\param key Klucz.
		\throw std::invalid_argument Jeżeli klucz jest pusty albo nie ma obiektu o takim kluczu.
		*/
		const T& get(const std::string& key) const;

		/** Zwróć referencję do obiektu o danym kluczu albo default_value, jeżeli takiego obiektu nie ma.
		\param key Klucz.
		\param default_value Domyślna wartość.
		*/
		const T& get(const std::string& key, const T& default_value) const;

		/// Zwróć liczbę obiektów w przechowalni.
		int size() const {
			return static_cast<int>(map_.size());
		}

		/// Sprawdź czy przechowalnia jest pusta.
		bool empty() const {
			return map_.empty();
		}
	private:
		std::unordered_map<std::string, T> map_;
	};

	template <class T> void Store<T>::add(const std::string& key, const T& value) {
		if (!has(key)) {
			map_.insert(std::make_pair(key, value));
		} else {
			throw std::invalid_argument((boost::locale::format("Key \"{1}\" already present") % key).str());
		}
	}

	template <class T> bool Store<T>::has(const std::string& key) const {
		if (key.empty()) {
			throw std::invalid_argument(boost::locale::translate("Key is empty"));
		}
		return map_.count(key) != 0;
	}

	template <class T> const T& Store<T>::get(const std::string& key) const {
		if (key.empty()) {
			throw std::invalid_argument(boost::locale::translate("Key is empty"));
		}
		const auto it = map_.find(key);
		if (it != map_.end()) {
			return it->second;
		} else {
			throw std::invalid_argument((boost::locale::format("No value with key \"{1}\"") % key).str());
		}
	}

	template <class T> const T& Store<T>::get(const std::string& key, const T& default_value) const {
		if (key.empty()) {
			throw std::invalid_argument(boost::locale::translate("Key is empty"));
		}
		const auto it = map_.find(key);
		if (it != map_.end()) {
			return it->second;
		} else {
			return default_value;
		}
	}
}
