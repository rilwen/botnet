#pragma once
#include <string>
#include <type_traits>
#include <boost/locale.hpp>

namespace simbo {
	/**
	\brief Funkcja kopiuje obiekt nie zmuszając programistów do wypisywania explicite nazwy typu T, która może być czasami bardzo długa.
	\tparam T Typ kopiowanego obiektu. Wymagany jest konstruktor kopiujący.
	*/
	template <class T> T make_copy(const T& obj) {
		// Niestety kompilator Visual Studio 2015 nie lubi polskich znaków diakrytycznych w asercjach.
		static_assert(std::is_copy_constructible<T>::value, "Typ musi byc kopiowalny");
		return T(obj);
	}

	/**
	\brief Znajdź maksymalny iterator it z przedziału [begin, end) taki, że *it <= value.

	\param begin Iterator do początku zakresu [begin, end).
	\param end Iterator do końca zakresu [begin, end).
	\param value Wyszukiwana wartość (referencja).
	\return Znaleziony iterator.
	\throws std::invalid_argument Jeżeli przedział [begin, end) jest pusty.
	\throws std::domain_error Jeżeli value jest mniejsza od *begin.
	*/
	template <class It, class V> It find_left_node(const It begin, const It end, const V& value) {
		if (begin == end) {
			throw std::invalid_argument(boost::locale::translate("Range is empty"));
		}
		auto it = std::lower_bound(begin, end, value);
		if (it == end) {
			--it;
			assert(value > *it);
			return it;
		}
		assert(value <= *it);
		if (value < *it) {
			if (it != begin) {
				--it;
			} else {
				throw std::domain_error(boost::locale::translate("Value below lower bound"));
			}
		}
		return it;
	}

	/**
	\brief Znajdź maksymalny iterator it z przedziału [begin, end) taki, że value <= *it.

	\param begin Iterator do początku zakresu [begin, end).
	\param end Iterator do końca zakresu [begin, end).
	\param value Wyszukiwana wartość (referencja).
	\return Znaleziony iterator.
	\throws std::invalid_argument Jeżeli przedział [begin, end) jest pusty
	\throws std::domain_error Jeżeli value jest większa od *(end - 1).
	*/
	template <class It, class V> It find_right_node(const It begin, const It end, const V& value) {
		if (begin == end) {
			throw std::invalid_argument(boost::locale::translate("Range is empty"));
		}
		auto it = std::lower_bound(begin, end, value);
		if (it == end) {
			throw std::domain_error(boost::locale::translate("Value above upper bound"));
		}
		assert(value <= *it);
		return it;
	}

	/** \brief Powtórz cyklicznie ciąg elementów tworząc wektor o podanej długości.
	Dla przykładu, repeat_elements([0, 1, 2], 5) == [0, 1, 2, 0, 1].
	*/
	template <class T> std::vector<T> repeat_elements(const std::vector<T>& elements, const int length) {
		const int number_elements = static_cast<int>(elements.size());
		if (!number_elements) {
			throw std::invalid_argument(boost::locale::translate("No elements"));
		}
		std::vector<T> result;
		result.reserve(length);
		for (int i = 0; i < length; ++i) {
			result.push_back(elements[i % number_elements]);
		}
		return result;
	}

	/// Znajdź rozszerzenie w nazwie pliku.
	/// \return Pusty łańcuch jeżeli nazwa nie ma rozszerzenia.
	std::string get_filename_extension(const std::string& filename);

	/// Sprawdź czy plik o danej nazwie istnieje.
	bool file_exists(const std::string& filename);
}
