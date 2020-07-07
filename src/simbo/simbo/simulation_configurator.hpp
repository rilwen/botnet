#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "antivirus.hpp"
#include "hasher.hpp"
#include "host.hpp"
#include "host_factory.hpp"
#include "json_fwd.hpp"
#include "local_network_factory.hpp"
#include "operating_system.hpp"
#include "output_config.hpp"
#include "simulation_config.hpp"

namespace simbo {

	class Context;
	class EmailProvider;
	class FixedHost;
	class IpAddressPool;
	class Schedule;
	class SimulationController;
	class World;

	/// Klasa konfigurująca symulację.
	class SimulationConfigurator {
	public:
		/// Domyślny konstruktor.
		SimulationConfigurator() {}

		/// Skonfiguruj symulację.
		/// \param j Dane JSON.
		SimulationController setup_simulation(const json& j);

		const OutputConfig& get_output_config() const {
			return output_config_;
		}
	private:
		/** \brief Wygeneruj kontekst symulacji na podstawie konfiguracji w formacie JSON. Zapisuje również sim_config_.
		Wymagane pola: "schedule" na harmonogram symulacji (klasa Schedule), "botnet_config" na konfigurację
		botnetu (klasa BotnetConfig), "simulation_config" na konfigurację symulacji (klasa SimulationConfig).
		\param j Dane JSON.
		\throw std::runtime_error Jeżeli dane JSON generują błędną konfigurację.
		*/
		Context parse_config_and_make_context(const json& j);

		World create_world(Context& ctx);

		uint64_t get_seed_from_clock() const;

		std::shared_ptr<IpAddressPool> make_ip_address_pool() const;

		void generate_local_networks(Context& ctx, World& world);

		void generate_email_providers();

		void generate_users(Context& ctx, World& world);

		void generate_operating_system_vendors();

		void generate_antivirus_vendors();

		Host::operating_system_ptr make_operating_system(os_full_spec_t os_full_spec);

		std::unique_ptr<Antivirus> make_antivirus(Antivirus::full_spec_t av_full_spec);

		void validate_operating_system(const os_full_spec_t& os_full_spec, const Schedule& schedule) const;

		/// Zapisz dane o strukturze wygenerowanego świata.
		void describe_world(const World& world) const;

		HostFactory host_factory_;
		LocalNetworkFactory local_network_factory_;
		SimulationConfig sim_config_;
		OutputConfig output_config_;

		typedef std::unordered_set<FixedHost*> fh_ptr_set;
		typedef std::vector<FixedHost*> fh_ptr_vec;

		fh_ptr_vec home_desktops_;	
		std::unordered_map<std::string, fh_ptr_vec> office_desktops_per_user_config_;

		std::vector<LocalNetwork*> corporate_local_networks_;
		std::unordered_map<std::string, std::shared_ptr<EmailProvider>> email_providers_;

		/// Kolekcja dostawców systemów operacyjnych.
		std::unordered_map<os_spec_t, std::shared_ptr<OperatingSystem::vendor_t>, Hasher> os_vendor_cache_;

		/// Kolekcja dostawców programów antywirusowych.
		std::unordered_map<Antivirus::spec_t, std::shared_ptr<Antivirus::vendor_t>, Hasher> av_vendor_cache_;
	};
}
