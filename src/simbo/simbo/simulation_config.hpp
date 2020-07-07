#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include "antivirus.hpp"
#include "antivirus_config.hpp"
#include "date_time.hpp"
#include "hasher.hpp"
#include "host_config.hpp"
#include "ip_address.hpp"
#include "json_fwd.hpp"
#include "local_network_config.hpp"
#include "operating_system.hpp"
#include "software_vendor_config.hpp"
#include "user_config.hpp"

namespace simbo {

	enum class LocalNetworkType;
	enum class OperatingSystemType;
	
	class Schedule;
	template <class T> class SparseDiscreteRandomVariable;

	/// Konfiguracja symulacji.
	struct SimulationConfig {		

		///// Konfiguracja sieci lokalnych.
		//struct NetworksConfig {			
		//	typedef std::pair<LocalNetworkType, int> local_network_type_size_t;
		//	typedef std::unordered_map<local_network_type_size_t, int, Hasher> local_network_count_map_t;

		//	/// Liczba sieci lokalnych o danym typie i rozmiarze.
		//	local_network_count_map_t local_networks;

		//	/** Mapuje typ sieci na konfiguracje hostów typu desktop. Musi zawierać konfiguracje dla wszystkich typów sieci w local_networks.
		//	*/
		//	std::unordered_map<LocalNetworkType, HostConfig> desktop_configs;

		//	/** Mapuje typ sieci na konfiguracje serwerów. Nie musi zawierać konfiguracji dla każdego typu sieci.
		//	*/
		//	std::unordered_map<LocalNetworkType, HostConfig> server_configs;

		//	/// Mapuje typ sieci na konfigurację sysadminów w tej sieci. Musi zawierać konfiguracje dla wszystkich typów sieci w local_networks.
		//	std::unordered_map<LocalNetworkType, SysadminConfig> desktop_sysadmin_configs;

		//	/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		//	/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		//	/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		//	void validate() const;
		//};

		/// Konfiguracja łączy publicznych.
		struct PublicConnectionsConfig {
			/// Minimalny adres IP.
			IpAddress min_ip_address;

			/// Maksymalny adres IP. Musi być tego samego typu i nie mniejszy niż min_ip_address.
			IpAddress max_ip_address;

			/// Czas życia dynamicznych IP. Musi być dodatni.
			seconds_t dynamic_ip_lifetime;

			/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
			/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
			/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
			void validate() const;
		};

		/// Konfiguracja poczty elektronicznej.
		struct EmailConfig {
			/// Mapuje ID providera -> efektywność filtrowania emaili. Zbiór kluczy definiuje wszystkich dostawców.
			std::unordered_map<std::string, double> filtering_efficiency;

			/// Rozkład prawdopodobieństwa dostawców poczty elektronicznej dla kont osobistych.
			std::shared_ptr<SparseDiscreteRandomVariable<std::string>> personal_email_provider;

			/// Rozkład prawdopodobieństwa dostawców poczty elektronicznej dla kont firmowych (jeden dostawca na sieć).
			std::shared_ptr<SparseDiscreteRandomVariable<std::string>> corporate_email_provider;

			/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
			/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
			/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
			void validate() const;
		};

		/// Konfiguracja użytkowników.
		struct UsersConfig {
			/// Rozkład par (rodzaj systemu operacyjnego, numer wersji) dla laptopów.
			HostConfig laptop_config;

			/// Konfiguracja sysadminów laptopów.
			SysadminConfig laptop_sysadmin_config;

			/// Konfiguracja każdego typu użytkownika.
			std::unordered_map<std::string, UserConfig> users;
			
			/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
			/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
			/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
			void validate() const;
		};

		/// Konfiguracja dostawców systemów operacyjnych.
		struct OperatingSystemVendorsConfig {
			/// Konfiguracja każdego dostawcy.
			std::unordered_map<std::string, OperatingSystem::vendor_t::config_t> vendors;

			/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
			/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
			/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
			void validate() const;
		};

		/// Konfiguracja antywirusów.
		struct AntivirusesConfig {
			/// Konfiguracja każdego dostawcy.
			std::unordered_map<std::string, Antivirus::vendor_t::config_t> vendors;

			/// Konfiguracja każdego antywirusa.
			std::unordered_map<Antivirus::spec_t, AntivirusConfig> antiviruses;

			/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
			/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
			/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
			void validate() const;
		};

		/// Ziarno dla generatora liczb losowych. Zerowe ziarno będzie interpretowane jako "użyj czas z zegaru systemowego".
		uint64_t random_seed;

		/// Konfiguracje sieci lokalnych.
		std::vector<LocalNetworkConfig> local_network_configs;

		/// Konfiguracja łączy publicznych.
		PublicConnectionsConfig public_connections_config;

		/// Konfiguracja poczty elektronicznej.
		EmailConfig email_config;

		/// Konfiguracja użytkowników.
		UsersConfig users_config;

		/// Konfiguracja dostawców systemów operacyjnych.
		OperatingSystemVendorsConfig operating_system_vendors_config;

		/// Konfiguracje antywirusów.
		AntivirusesConfig antiviruses_config;

		/// Rzuć wyjątkiem, jeżeli konfiguracja jest niepoprawna.
		/// \throw std::domain_error Jeżeli wartość parametru jest spoza udokumentowanej dziedziny.
		/// \throw std::invalid_argument Jeżeli wartość parametru nie spełnia innego warunku.
		void validate() const;
	};

	/// Odczytaj obiekt SimulationConfig z formatu JSON.
	/// Po odczytaniu, woła funkcję SimulationConfig#validate().
	void from_json(const json& j, const Schedule& schedule, SimulationConfig& config);
}
