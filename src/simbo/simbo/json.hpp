#pragma once
#include <cassert>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <nlohmann/json.hpp>
#include "exceptions.hpp"
#include "store.hpp"

using json = nlohmann::json;

namespace simbo {
	/// Sprawdź czy dane JSON opisują obiekt i rzuć wyjątkiem, jeśli nie.
	/// \param j Dane JSON.
	/// \param target_class Nazwa klasy ktorą chcemy deserializować z JSON.
	/// \throw DeserialisationError Jeżeli j nie jest obiektem JSON.
	void expect_object(const json& j, const std::string& target_class);

	/// Sprawdź czy klucz jest obecny w obiekcie JSON i rzuć wyjątkiem jeśli nie jest.
	/// \param j Dane JSON.
	/// \param target_class Nazwa klasy ktorą chcemy deserializować z JSON.
	/// \param key Klucz.
	/// \throw DeserialisationError Jeżeli klucz key nie jest obecny w obiekcie j.
	void expect_key(const json& j, const std::string& target_class, const std::string& key);

	/// Sprawdź klucze w obiekcie.
	/// \param j Dane JSON.
	/// \param target_class Nazwa klasy ktorą chcemy deserializować z JSON.
	/// \param expected_keys Lista wymaganych kluczy.
	/// \param optional_keys Lista opcjonalnych kluczy.
	/// \throw DeserialisationError Jeżeli brakuje klucza z listy expected_keys. Jeżeli w obiekcie jest klucz którego nie ma w żadnej z dwóch podanych list. Jeżeli j nie jest obiektem.
	void validate_keys(const json& j, const std::string& target_class, std::initializer_list<std::string> expected_keys,
		std::initializer_list<std::string> optional_keys);

	/// Zwróć wartość klucza key z obiektu JSON jeżeli zawiera on ten klucz, w przeciwnym wypadku zwróć default_value.
	/// \param j Dane JSON.
	/// \param key Klucz.
	/// \param default_value Domyślna wartość.
	/// \tparam T Typ wartości.
	template <class T> T get_value(const json& j, const std::string& key, const T& default_value) {
		if (j.find(key) != j.end()) {
			return j[key];
		} else {
			return default_value;
		}
	}

	inline void from_string(const std::string& src, std::string& dst) {
		dst = src;
	}

	template <class K, class V, class H> void get_map_from_object(const json& j, const std::string& target_class, std::unordered_map<K, V, H>& map) {
		expect_object(j, target_class);
		try {
			map.clear();
			for (auto it = j.begin(); it != j.end(); ++it) {
				K key;
				from_string(it.key(), key);
				const V value = it.value();
				map.insert(std::make_pair(std::move(key), std::move(value)));
			}
		} catch (const std::exception& e) {
			throw DeserialisationError(target_class, j.dump(), e);
		}
	}

	template <class K, class V, class A, class H> void get_map_from_object(const json& j, const std::string& target_class, A& aux, std::unordered_map<K, V, H>& map) {
		expect_object(j, target_class);
		try {
			map.clear();
			for (auto it = j.begin(); it != j.end(); ++it) {
				K key;
				from_string(it.key(), key);
				V value;
				from_json(it.value(), aux, value);
				map.insert(std::make_pair(std::move(key), std::move(value)));
			}
		} catch (const std::exception& e) {
			throw DeserialisationError(target_class, j.dump(), e);
		}
	}

	template <class Derived, class Base>
	std::enable_if_t<std::is_base_of_v<Base, Derived>> deserialise_derived(const json& j, std::unique_ptr<Base>& base_ptr, const std::string& base_type, const std::string& json_type) {
		std::unique_ptr<Derived> derived_ptr;
		try {
			from_json(j, derived_ptr);
		} catch (const DeserialisationError&) {
			throw;
		} catch (const std::exception& e) {
			throw DeserialisationError(base_type, j.dump(), e);
		}
		base_ptr = std::move(derived_ptr);
	}

	template <class Derived, class Base>
	std::enable_if_t<std::negation_v<std::is_base_of<Base, Derived>>> deserialise_derived(const json& j, std::unique_ptr<Base>& base_ptr, const std::string& base_type, const std::string& json_type) {
		throw DeserialisationError(base_type, j.dump(), (boost::locale::format("Deserialised {1} type \"{2}\" incompatible with target C++ type") % base_type % json_type).str());
	}

	template <class Derived, class A, class Base>
	std::enable_if_t<std::is_base_of_v<Base, Derived>> deserialise_derived(const json& j, A& aux, std::unique_ptr<Base>& base_ptr, const std::string& base_type, const std::string& json_type) {
		std::unique_ptr<Derived> derived_ptr;
		from_json(j, aux, derived_ptr);
		base_ptr = std::move(derived_ptr);
	}

	template <class Derived, class A, class Base>
	std::enable_if_t<std::negation_v<std::is_base_of<Base, Derived>>> deserialise_derived(const json& j, A& aux, std::unique_ptr<Base>& base_ptr, const std::string& base_type, const std::string& json_type) {
		throw DeserialisationError(base_type, j.dump(), (boost::locale::format("Deserialised {1} type \"{2}\" incompatible with target C++ type") % base_type % json_type).str());
	}

	/** \brief Odczytaj obiekt z JSON, bezpośrednio albo poprzez odwołanie do poprzednio odczytanej definicji.
	\param j Dane JSON.
	\param store Przechowalnia obiektów.
	\param value Referencja w której zostanie zapisany obiekt.
	\throw DeserialisationError Jeżeli odczytanie się nie powiedzie.
	*/
	template <class T> void from_json_or_store(const json& j, const Store<T>& store, T& value) {
		if (j.is_string()) {
			const std::string key = j;
			try {
				value = store.get(key);
			} catch (std::exception& e) {
				throw DeserialisationError(typeid(T).name(), j.dump(), e);
			}
		} else if (j.is_object()) {
			from_json(j, value);
		} else {
			const char* type_name = typeid(T).name();
			throw DeserialisationError(type_name, j.dump(), (boost::locale::format("Only in-line definitions or store keys allowed when deserialising to type {1}") % type_name).str());
		}
	}

	/** \brief Odczytaj obiekt z JSON, bezpośrednio albo poprzez odwołanie do poprzednio odczytanej definicji.
	\param j Dane JSON.
	\param aux Obiekt pomocniczy.
	\param store Przechowalnia obiektów.
	\param value Referencja w której zostanie zapisany obiekt.
	\throw DeserialisationError Jeżeli odczytanie się nie powiedzie.
	\tparam A Typ obiektu pomocniczego.
	*/
	template <class A, class T> void from_json_or_store(const json& j, A& aux, const Store<T>& store, T& value) {
		if (j.is_string()) {
			const std::string key = j;
			try {
				value = store.get(key);
			} catch (std::exception& e) {
				throw DeserialisationError(typeid(T).name(), j.dump(), e);
			}
		} else if (j.is_object()) {
			from_json(j, aux, value);
		} else {
			const char* type_name = typeid(T).name();
			throw DeserialisationError(type_name, j.dump(), (boost::locale::format("Only in-line definitions or store keys allowed when deserialising to type {1}") % type_name).str());
		}
	}

	/// Odczytaj wskaźnik unique_ptr do const T z formatu JSON.
	/// \throw DeserialisationError
	template <class T> void from_json(const json& j, std::unique_ptr<const T>& const_unique_ptr) {
		std::unique_ptr<T> unique_ptr;
		from_json(j, unique_ptr);
		assert(unique_ptr);
		const_unique_ptr = std::move(unique_ptr);
	}

	/// Odczytaj wskaźnik shared_ptr do T z formatu JSON.
	/// \throw DeserialisationError
	template <class T> void from_json(const json& j, std::shared_ptr<T>& shared_ptr) {
		std::unique_ptr<T> unique_ptr;
		from_json(j, unique_ptr);
		assert(unique_ptr);
		shared_ptr = std::move(unique_ptr);
	}

	/// Odczytaj wskaźnik shared_ptr do const T z formatu JSON.
	/// \throw DeserialisationError
	template <class T> void from_json(const json& j, std::shared_ptr<const T>& const_shared_ptr) {
		std::unique_ptr<T> unique_ptr;
		from_json(j, unique_ptr);
		assert(unique_ptr);
		const_shared_ptr = std::move(unique_ptr);
	}

	/// Odczytaj wskaźnik shared_ptr do const T z formatu JSON, korzystając z pomocniczego obiektu.
	/// \throw DeserialisationError
	template <class A, class T> 
	void from_json(const json& j, A& aux, std::shared_ptr<const T>& const_shared_ptr) {
		std::unique_ptr<T> unique_ptr;
		from_json(j, aux, unique_ptr);
		assert(unique_ptr);
		const_shared_ptr = std::move(unique_ptr);
	}

	/// Odczytaj wskaźnik shared_ptr do const T z formatu JSON, korzystając z pomocniczego obiektu.
	/// \throw DeserialisationError
	template <class A, class T>
	void from_json(const json& j, A& aux, std::shared_ptr<T>& shared_ptr) {
		std::unique_ptr<T> unique_ptr;
		from_json(j, aux, unique_ptr);
		assert(unique_ptr);
		shared_ptr = std::move(unique_ptr);
	}

	/// Odczytaj mapę K -> V z formatu JSON, korzystając z pomocniczego obiektu.
	template <class A, class K, class V> void from_json(const json& j, A& aux, std::unordered_map<K, V>& map) {
		const char* key_name = typeid(K).name();
		const char* value_name = typeid(V).name();
		const std::string type_name = (boost::locale::format("std::unordered_map<{1}, {2}>") % key_name % value_name).str();
		if (j.is_array()) {
			for (const auto& elem : j) {
				if (elem.is_array() && elem.size() == 2) {
					const K key = elem[0];
					V value;
					from_json(elem[1], aux, value);
					map[key] = value;
				} else {
					throw DeserialisationError(type_name, j.dump(), (boost::locale::format("Only size 2 arrays allowed as elements when deserialising to type {1}") % type_name).str());
				}
			}
		} else {
			throw DeserialisationError(type_name, j.dump(), (boost::locale::format("Only arrays allowed when deserialising to type {1}") % type_name).str());
		}
	}

	/// Odczytaj wektor elementów V z formatu JSON, korzystając z pomocniczego obiektu.
	template <class A, class V> void from_json(const json& j, A& aux, std::vector<V>& vec) {
		const char* value_name = typeid(V).name();
		const std::string type_name = (boost::locale::format("std::vector<{1}>") % value_name).str();
		if (j.is_array()) {
			for (const auto& elem : j) {
				V value;
				from_json(elem, aux, value);
				vec.push_back(value);
			}
		} else {
			throw DeserialisationError(type_name, j.dump(), (boost::locale::format("Only arrays allowed when deserialising to type {1}") % type_name).str());
		}
	}
}
