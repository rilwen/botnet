#pragma once
#include <unordered_map>
#include <unordered_set>
#include "host_config.hpp"
#include "host_type.hpp"
#include "json_fwd.hpp"
#include "local_network_type.hpp"
#include "sysadmin_config.hpp"

namespace simbo {
	class Schedule;

	/// Konfiguracja sieci lokalnej.
	struct LocalNetworkConfig {

		/// Konfiguracje hostów stałych danego typu w sieci. Klucze ograniczone są do typów hostów stałych.
		/// Wartości to pary (konfiguracja hosta, liczba hostów). Liczby hostów muszą być dodatnie.
		std::unordered_map<HostType, std::pair<HostConfig, int>> fixed_hosts;

		/// Zbiór zestawów konfiguracji użytkowników korzystających z danej sieci.
		/// Konfiguracje muszą być zdefiniowane w strukturze UsersConfig.
		std::unordered_set<std::string> allowed_users;

		/// Konfiguracja sysadminów sieci.
		SysadminConfig sysadmin_config;

		/// Liczba sieci o tej konfiguracji. Musi być dodatnia.
		int count;

		/// Typ sieci lokalnej.
		LocalNetworkType typ;

		/// Kod kraju, w przedziale [0, 255]. Wartość dodatnia oznacza numer kraju na liście posortowanych dwuliterowych kodów ISO. Wartość zero oznacza brak informacji.
		int country;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt LocalNetworkConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję UserConfig#validate().
	void from_json(const json& j, const Schedule& schedule, LocalNetworkConfig& config);
}
