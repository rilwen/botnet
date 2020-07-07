#include <ctime>
#include <fstream>
#include <boost/format.hpp>
#include "botnet.hpp"
#include "context.hpp"
#include "desktop.hpp"
#include "email_provider.hpp"
#include "exceptions.hpp"
#include "host_config.hpp"
#include "json.hpp"
#include "log.hpp"
#include "operating_system.hpp"
#include "pausing_jump_markov_process.hpp"
#include "public_connection_dynamic_address.hpp"
#include "public_connection_static_address.hpp"
#include "rng_impl.hpp"
#include "server.hpp"
#include "simple_poisson_process.hpp"
#include "simulation_config.hpp"
#include "simulation_configurator.hpp"
#include "simulation_controller.hpp"
#include "sysadmin_config.hpp"
#include "sparse_discrete_random_variable.hpp"
#include "unique_ip_address_pool.hpp"
#include "world.hpp"

namespace simbo {
	SimulationController SimulationConfigurator::setup_simulation(const json& j) {		
		validate_keys(j, "SimulationController", { "schedule", "botnet_config", "simulation_config", "output_config" }, {});
		try {
			output_config_ = j["output_config"];
		} catch (const std::exception& e) {
			throw DeserialisationError("OutputConfig", j.dump(), e);
		}
		Context context(parse_config_and_make_context(j));
		get_logger()->info("SimulationConfigurator: Created simulation context.");
		World world(create_world(context));
		get_logger()->info("SimulationConfigurator: Created {} email accounts", world.get_number_email_accounts());
		if (context.get_botnet_config().email_address_list_size < world.get_number_email_accounts()) {
			throw ConfigurationError("Simulation", (boost::locale::format("Email address list size ({1}) is lower than total number of email accounts in the simulation world ({2})") % context.get_botnet_config().email_address_list_size % world.get_number_email_accounts()).str());
		}
		return SimulationController(std::move(world), std::move(context), output_config_);
	}

	Context SimulationConfigurator::parse_config_and_make_context(const json& j) {
		expect_key(j, "Context", "schedule");
		expect_key(j, "Context", "botnet_config");
		expect_key(j, "Context", "simulation_config");
		try {
			Schedule schedule = j["schedule"];
			from_json(j["simulation_config"], schedule, sim_config_);
			const BotnetConfig botnet_config = j["botnet_config"];
			for (const BotnetPayloadConfig& payload_config : botnet_config.payload.get_ys()) {
				for (const auto& kv : payload_config.min_detecting_av_release) {
					const auto& av_spec = kv.first;
					if (!sim_config_.antiviruses_config.antiviruses.count(av_spec)) {
						throw ConfigurationError("Context", (boost::locale::format("No configuration for antivirus {1}") % av_spec).str());
					}
				}
			}			
			for (const os_full_spec_t& os_full_spec : sim_config_.users_config.laptop_config.operating_system->get_values()) {
				validate_operating_system(os_full_spec, schedule);
			}
			for (const LocalNetworkConfig& ln_cfg: sim_config_.local_network_configs) {
				for (const auto& host_typ_and_cfg_and_count : ln_cfg.fixed_hosts) {
					const HostConfig& host_cfg = host_typ_and_cfg_and_count.second.first;
					const auto os_rv = host_cfg.operating_system;
					assert(os_rv);
					for (const os_full_spec_t& os_full_spec : os_rv->get_values()) {
						validate_operating_system(os_full_spec, schedule);
					}
				}				
			}

			const bool save_full_data = !output_config_.sinkhole_data_filename.empty();
			std::unique_ptr<Botnet> botnet(new Botnet(botnet_config, Sinkhole(schedule.get_start(), save_full_data)));
			uint64_t random_seed = sim_config_.random_seed;
			if (!random_seed) {
				random_seed = get_seed_from_clock();
				get_logger()->info("SimulationConfigurator: using clock-generated random seed: {}.", random_seed);
			}
			const std::shared_ptr<RNG> rng(new RNGImpl(random_seed));
			return Context(rng, std::move(botnet), std::move(schedule));
		} catch (const DeserialisationError& e) {
			throw e;
		} catch (const std::exception& e) {
			throw DeserialisationError("Context", j.dump(), e);
		}
	}

	uint64_t SimulationConfigurator::get_seed_from_clock() const {
		return time(0);
	}

	std::shared_ptr<IpAddressPool> SimulationConfigurator::make_ip_address_pool() const {
		const auto& pc_config = sim_config_.public_connections_config;
		if (pc_config.min_ip_address.is_v4()) {
			return std::shared_ptr<IpAddressPool>(new UniqueIpAddressPoolV4(pc_config.min_ip_address.to_v4(), pc_config.max_ip_address.to_v4()));
		} else {
			return std::shared_ptr<IpAddressPool>(new UniqueIpAddressPoolV6(pc_config.min_ip_address.to_v6(), pc_config.max_ip_address.to_v6()));
		}
	}

	Host::operating_system_ptr SimulationConfigurator::make_operating_system(os_full_spec_t os_full_spec) {
		const auto os_spec = os_full_spec.first;
		const auto vendor_it = os_vendor_cache_.find(os_spec);
		if (vendor_it == os_vendor_cache_.end()) {
			throw ConfigurationError("OperatingSystem", (boost::locale::format("No vendor for operating system type {1} and version {2}") % os_spec.first % os_spec.second).str());
		}
		const int release = os_full_spec.second;
		return OperatingSystem::build(vendor_it->second, os_spec, release);
	}	

	std::unique_ptr<Antivirus> SimulationConfigurator::make_antivirus(Antivirus::full_spec_t av_full_spec) {
		const auto av_spec = av_full_spec.first;
		if (!Antivirus::is_null(av_spec)) {
			const auto vendor_it = av_vendor_cache_.find(av_spec);
			if (vendor_it == av_vendor_cache_.end()) {
				throw ConfigurationError("Antivirus", (boost::locale::format("No vendor for antivirus {1}") % av_spec).str());
			}
			const int release = av_full_spec.second;
			const AntivirusConfig& av_config = sim_config_.antiviruses_config.antiviruses.find(av_spec)->second;
			return std::make_unique<Antivirus>(av_spec, vendor_it->second, release, av_config);
		} else {
			// Pusta nazwa antywirusa w konfiguracji oznacza brak antywirusa.
			return nullptr;
		}
	}

	void SimulationConfigurator::generate_local_networks(Context& ctx, World& world) {
		// Tylko 1 ISP.		 
		const std::shared_ptr<IpAddressPool> ip_address_pool = make_ip_address_pool();
		world.add_isp(InternetServiceProvider(ip_address_pool));
		InternetServiceProvider& isp = *(world.get_isps_begin());
		get_logger()->info("SimulationConfigurator: Created single ISP.");
		
		const std::vector<LocalNetworkConfig>& net_configs = sim_config_.local_network_configs;
		const auto& pc_config = sim_config_.public_connections_config;
		
		int total_nbr_fixed_hosts = 0;
		int total_nbr_networks = 0;
		int total_nbr_fixed_hosts_with_antivirus = 0;
		int nbr_home_servers = 0;
		int nbr_office_servers = 0;
		int nbr_home_fixed_hosts = 0;
		int nbr_office_fixed_hosts = 0;

		// Stwórz sieci lokalne.
		for (const LocalNetworkConfig& ln_cfg : net_configs) {
			// Typ sieci.
			const LocalNetworkType ln_type = ln_cfg.typ;
			// Liczba sieci o danej konfiguracji.
			const int count = ln_cfg.count;
			total_nbr_networks += count;

			for (int i = 0; i < count; ++i) {
				LocalNetwork* ln_ptr = nullptr;

				{
					NetworkAdmin network_admin;
					ln_ptr = local_network_factory_.make_local_network(network_admin, ln_type, ln_cfg.country);
					world.add_network_admin(std::move(network_admin));
				}
				assert(ln_ptr != nullptr);
				
				{
					std::unique_ptr<PublicConnection> public_connection;
					if (ln_type == LocalNetworkType::HOUSEHOLD) {
						public_connection.reset(new PublicConnectionDynamicAddress(ip_address_pool, pc_config.dynamic_ip_lifetime, 0));
					} else {
						if (ln_type == LocalNetworkType::CORPORATE) {
							corporate_local_networks_.push_back(ln_ptr);
						}
						public_connection.reset(new PublicConnectionStaticAddress(ip_address_pool->allocate()));
					}
					ln_ptr->set_public_connection(public_connection.get());
					isp.add_connection(std::move(public_connection));
				}

				Sysadmin sysadmin((*ln_cfg.sysadmin_config.maintenance_triggers)(ctx.get_rng()));

				for (const auto& host_typ_and_cfg_and_count : ln_cfg.fixed_hosts) {
					// Liczba hostów danego typu w sieci.
					const HostType host_type = host_typ_and_cfg_and_count.first;
					assert(is_host_type_fixed(host_type));
					const std::pair<HostConfig, int>& host_config_and_count = host_typ_and_cfg_and_count.second;
					const HostConfig& host_config = host_config_and_count.first;
					const int nbr_hosts = host_config_and_count.second;
					
					total_nbr_fixed_hosts += nbr_hosts;
					switch (ln_type) {
					case LocalNetworkType::HOUSEHOLD:
						nbr_home_fixed_hosts += nbr_hosts;
						if (host_type == HostType::SERVER) {
							nbr_home_servers += nbr_hosts;
						}						
						break;
					case LocalNetworkType::CORPORATE:
						nbr_office_fixed_hosts += nbr_hosts;
						if (host_type == HostType::SERVER) {
							nbr_office_servers += nbr_hosts;
						}
						break;
					default:
						throw ConfigurationError("LocalNetwork",
							(boost::locale::format("Unsupported local network type {1}") % ln_type).str());
					}					
					
					for (int j = 0; j < nbr_hosts; ++j) {
						const os_full_spec_t os_full_spec = (*host_config.operating_system)(ctx.get_rng());
						auto os = make_operating_system(os_full_spec);
						assert(os);
						FixedHost* host;
						switch (host_type) {
						case HostType::DESKTOP:
							host = host_factory_.make_desktop(sysadmin, std::move(os), false, ln_ptr);
							break;
						case HostType::SERVER:
							host = host_factory_.make_server(sysadmin, std::move(os), ln_ptr);
							break;
						default:
							throw ConfigurationError("LocalNetwork",
								(boost::locale::format("Unsupported fixed host type {1}") % host_type).str());
						}
						assert(host);
						const auto av_rv_it = host_config.antivirus.find(os_full_spec.first);
						if (av_rv_it != host_config.antivirus.end()) {
							const Antivirus::full_spec_t av_full_spec = (*av_rv_it->second)(ctx.get_rng());
							host->set_antivirus(make_antivirus(av_full_spec));
							++total_nbr_fixed_hosts_with_antivirus;
						}
						if (host_type == HostType::DESKTOP) {
							if (ln_type == LocalNetworkType::HOUSEHOLD) {
								home_desktops_.push_back(host);								
							} else if (ln_type == LocalNetworkType::CORPORATE) {								
								for (const auto& user_cfg_name : ln_cfg.allowed_users) {
									office_desktops_per_user_config_[user_cfg_name].push_back(host);
								}
							}
						}
					}
				}

				world.add_sysadmin(std::move(sysadmin));
			}
		}		
		const int total_nbr_servers = nbr_home_servers + nbr_office_servers;
		get_logger()->info("SimulationConfigurator: Created {} local networks, including {} corporate ones.", total_nbr_networks, corporate_local_networks_.size());
		get_logger()->info("SimulationConfigurator: Created {} fixed hosts, including {} corporate ones and {} home ones.", total_nbr_fixed_hosts, nbr_office_fixed_hosts, nbr_home_fixed_hosts);
		get_logger()->info("SimulationConfigurator: Created {} servers, including {} corporate ones and {} home ones.", total_nbr_servers, nbr_office_servers, nbr_home_servers);
		get_logger()->info("SimulationConfigurator: {} fixed hosts have an antivirus program.", total_nbr_fixed_hosts_with_antivirus);
	}

	void SimulationConfigurator::generate_users(Context& ctx, World& world) {
		const auto& users_config = sim_config_.users_config;
		RNG::StlView stl_rng(ctx.get_rng());

		// Przydziel każdej sieci korporacyjnej dostawcę email.
		std::unordered_map<const LocalNetwork*, std::shared_ptr<EmailProvider>> corporate_email_providers;
		for (LocalNetwork* corporate_ln : corporate_local_networks_) {
			corporate_email_providers[corporate_ln] = email_providers_[(*sim_config_.email_config.corporate_email_provider)(ctx.get_rng())];
		}
		get_logger()->debug((boost::format("SimulationConfiguration: Assigned email providers to %d corporate networks.") % corporate_local_networks_.size()).str());

		/// Iteratory używane do losowego wyboru sieci z wagami proporcjonalnymi do liczby hostów w sieci.

		// Ustaw iterator na koniec wektora, żeby wymusić losowe przemieszanie wektorów w pierwszej iteracji pętli.		
		auto home_host_iter = home_desktops_.end();
		auto home_host_iter_for_network_choice = home_host_iter;

		std::unordered_map<std::string, fh_ptr_vec::iterator> office_host_iters;				
		std::unordered_map<std::string, fh_ptr_vec::iterator> office_host_iters_for_network_choice;

		for (const auto& user_type_and_cfg : users_config.users) {
			const auto& cfg_name = user_type_and_cfg.first;
			if (!office_desktops_per_user_config_.count(cfg_name)) {
				office_desktops_per_user_config_.insert(std::make_pair(cfg_name, fh_ptr_vec()));
			}
			std::shuffle(office_desktops_per_user_config_[cfg_name].begin(), office_desktops_per_user_config_[cfg_name].end(), stl_rng);
			office_host_iters[cfg_name] = office_desktops_per_user_config_[cfg_name].begin();			
			office_host_iters_for_network_choice[cfg_name] = office_host_iters[cfg_name];
		}		

		int nbr_laptops = 0;
		int nbr_laptops_with_antivirus = 0;

		for (const auto& user_type_and_cfg : users_config.users) {
			const auto& cfg_name = user_type_and_cfg.first;
			const auto& user_cfg = user_type_and_cfg.second;
			get_logger()->debug((boost::format("SimulationConfigurator: Creating %d users of type %s.") % user_cfg.count % cfg_name).str());
			
			// Sprawdź gdzie użytkownik korzysta z komputerów.
			std::array<bool, EnumNames<UserEnvironmentType>::NAMES.size()> uses_environment;
			std::fill(uses_environment.begin(), uses_environment.end(), false);
			for (const UserEnvironmentType ue_typ : user_cfg.environments) {
				uses_environment[static_cast<int>(ue_typ)] = true;
			}
			int nbr_environments_used = 0;
			for (bool flag : uses_environment) {
				if (flag) {
					++nbr_environments_used;
				}
			}
			get_logger()->debug((boost::format("SimulationConfigurator: User %s uses %d environments.") % cfg_name % nbr_environments_used).str());
			
			auto& office_desktops = office_desktops_per_user_config_[cfg_name];
			// Muszą być referencje, nie kopie.
			auto& office_host_iter = office_host_iters[cfg_name];
			auto& office_host_iter_for_network_choice = office_host_iters_for_network_choice[cfg_name];

			for (int i = 0; i < user_cfg.count; ++i) {
				if (home_host_iter == home_desktops_.end()) {
					std::shuffle(home_desktops_.begin(), home_desktops_.end(), stl_rng);
					get_logger()->debug("SimulationConfigurator: Reshuffled home desktops.");
					home_host_iter = home_desktops_.begin();
					// Uaktualnij drugi iterator.
					home_host_iter_for_network_choice = home_host_iter;
				}
				if ((std::find(user_cfg.environments.begin(), user_cfg.environments.end(), UserEnvironmentType::OFFICE) != user_cfg.environments.end()) &&
					!user_cfg.uses_laptop
					) {
					if (office_host_iter == office_desktops.end()) {
						throw ConfigurationError("Users", (boost::locale::format("Not enough fixed hosts in corporate networks for user configuration {1}") % cfg_name).str());
					}
				}
				if (office_host_iter_for_network_choice == office_desktops.end()) {
					office_host_iter_for_network_choice = office_desktops.begin();
				}
				if (home_host_iter_for_network_choice == home_desktops_.end()) {
					home_host_iter_for_network_choice = home_desktops_.begin();
				}

				std::array<Host*, EnumNames<UserEnvironmentType>::NAMES.size()> hosts;
				std::fill(hosts.begin(), hosts.end(), nullptr);

				std::array<EmailAccount::email_provider_ptr, EnumNames<UserEnvironmentType>::NAMES.size()> email_providers;
				std::fill(email_providers.begin(), email_providers.end(), nullptr);

				email_providers[static_cast<int>(UserEnvironmentType::HOME)] = email_providers_[(*sim_config_.email_config.personal_email_provider)(ctx.get_rng())];

				EmailAccount::local_networks_set personal_email_local_networks;
				EmailAccount::local_networks_set work_email_local_networks;

				if (user_cfg.uses_laptop) {
					// Użytkownik ma tylko 1 komputer.
					Laptop* laptop = nullptr;
					
					std::array<LocalNetwork*, EnumNames<UserEnvironmentType>::NAMES.size()> networks;
					std::fill(networks.begin(), networks.end(), nullptr);

					// Wybierz sieci w których używany będzie laptop.

					std::vector<LocalNetwork*> used_networks;
					int initial_network_index = 0;
					
					used_networks.reserve(nbr_environments_used);
					for (const UserEnvironmentType ue_typ : user_cfg.environments) {
						if (ue_typ == UserEnvironmentType::HOME) {
							// *(iterator++) zwraca wartość wskazywaną przez iterator a potem go przesuwa.
							if (!networks[static_cast<int>(UserEnvironmentType::HOME)]) {
								// Alternatywą dla const_cast byłoby zwracanie przez Host wskaźnika nie-const do
								// LocalNetwork (osłabienie enkapsulacji) albo kosztowniejszy i mniej dokładny numerycznie
								// sposób losowania sieci dla laptopów.
								networks[static_cast<int>(UserEnvironmentType::HOME)] = const_cast<LocalNetwork*>((*home_host_iter_for_network_choice++)->get_local_network());
							}
							used_networks.push_back(networks[static_cast<int>(UserEnvironmentType::HOME)]);
						} else if (ue_typ == UserEnvironmentType::OFFICE) {
							if (!networks[static_cast<int>(UserEnvironmentType::OFFICE)]) {
								networks[static_cast<int>(UserEnvironmentType::OFFICE)] = const_cast<LocalNetwork*>((*office_host_iter_for_network_choice++)->get_local_network());
								email_providers[static_cast<int>(UserEnvironmentType::OFFICE)] = corporate_email_providers[networks[static_cast<int>(UserEnvironmentType::OFFICE)]];
							}
							used_networks.push_back(networks[static_cast<int>(UserEnvironmentType::OFFICE)]);
						} else if (ue_typ == UserEnvironmentType::NONE) {
							initial_network_index = static_cast<int>(used_networks.size());
							used_networks.push_back(nullptr);
						}
					}

					for (const UserEnvironmentType ue_typ : user_cfg.personal_email_environments) {
						assert(ue_typ != UserEnvironmentType::NONE);
						personal_email_local_networks.insert(networks[static_cast<int>(ue_typ)]);
					}
					for (const UserEnvironmentType ue_typ : user_cfg.work_email_environments) {
						assert(ue_typ != UserEnvironmentType::NONE);
						work_email_local_networks.insert(networks[static_cast<int>(ue_typ)]);
					}

					LocalNetwork* initial_network = used_networks[initial_network_index];
					Laptop::local_network_switcher_t network_switcher(user_cfg.environment_process, std::move(used_networks), initial_network);
					const auto os_full_spec = (*users_config.laptop_config.operating_system)(ctx.get_rng());
					auto os = make_operating_system(os_full_spec);
					assert(os);
					// Użytkownik = sysadmin.
					Sysadmin sysadmin((*users_config.laptop_sysadmin_config.maintenance_triggers)(ctx.get_rng()));
					laptop = host_factory_.make_laptop(sysadmin, std::move(os), std::move(network_switcher));
					world.add_sysadmin(std::move(sysadmin));
					assert(laptop);
					const auto av_rv_it = users_config.laptop_config.antivirus.find(os_full_spec.first);
					if (av_rv_it != users_config.laptop_config.antivirus.end()) {
						const Antivirus::full_spec_t av_full_spec = (*av_rv_it->second)(ctx.get_rng());
						laptop->set_antivirus(make_antivirus(av_full_spec));
						++nbr_laptops_with_antivirus;
					}
					// Użytkownik robi wszystko na laptopie.
					hosts[static_cast<int>(UserEnvironmentType::HOME)] = laptop;
					hosts[static_cast<int>(UserEnvironmentType::OFFICE)] = laptop;
					++nbr_laptops;
				}

				// Wybierz hosty używane przez użytkowniczkę.
				EmailAccount::hosts_set personal_email_hosts;
				EmailAccount::hosts_set work_email_hosts;
				std::vector<Host*> used_hosts;
				used_hosts.reserve(nbr_environments_used);
				int initial_host_index = 0;
				for (const UserEnvironmentType ue_typ : user_cfg.environments) {
					if (ue_typ == UserEnvironmentType::HOME) {
						if (!hosts[static_cast<int>(UserEnvironmentType::HOME)]) {
							hosts[static_cast<int>(UserEnvironmentType::HOME)] = *(home_host_iter++);
							assert(hosts[static_cast<int>(UserEnvironmentType::HOME)]);
						}
						used_hosts.push_back(hosts[static_cast<int>(UserEnvironmentType::HOME)]);
					} else if (ue_typ == UserEnvironmentType::OFFICE) {
						if (!hosts[static_cast<int>(UserEnvironmentType::OFFICE)]) {
							hosts[static_cast<int>(UserEnvironmentType::OFFICE)] = *(office_host_iter++);
							assert(hosts[static_cast<int>(UserEnvironmentType::OFFICE)]);
						}
						if (!email_providers[static_cast<int>(UserEnvironmentType::OFFICE)]) {
							email_providers[static_cast<int>(UserEnvironmentType::OFFICE)] = corporate_email_providers[hosts[static_cast<int>(UserEnvironmentType::OFFICE)]->get_local_network()];
						}
						used_hosts.push_back(hosts[static_cast<int>(UserEnvironmentType::OFFICE)]);
					} else if (ue_typ == UserEnvironmentType::NONE) {
						initial_host_index = static_cast<int>(used_hosts.size());
						used_hosts.push_back(nullptr);
					}
				}

				for (const UserEnvironmentType ue_typ : user_cfg.personal_email_environments) {
					assert(ue_typ != UserEnvironmentType::NONE);
					personal_email_hosts.insert(hosts[static_cast<int>(ue_typ)]);
				}
				for (const UserEnvironmentType ue_typ : user_cfg.work_email_environments) {
					assert(ue_typ != UserEnvironmentType::NONE);
					work_email_hosts.insert(hosts[static_cast<int>(ue_typ)]);
				}

				Host* initial_host = used_hosts[initial_host_index];

				User::email_accounts_vec email_accounts;

				// Zawsze tworzymy osobiste konto e-mail.
				email_accounts.push_back(EmailAccount(email_providers[static_cast<int>(UserEnvironmentType::HOME)], user_cfg.personal_email_check_process, std::move(personal_email_hosts), std::move(personal_email_local_networks)));
				if (email_providers[static_cast<int>(UserEnvironmentType::OFFICE)]) {
					email_accounts.push_back(EmailAccount(email_providers[static_cast<int>(UserEnvironmentType::OFFICE)], user_cfg.work_email_check_process, std::move(work_email_hosts), std::move(work_email_local_networks)));
				}
				User::host_switcher_t host_switcher(user_cfg.environment_process, std::move(used_hosts), initial_host);
				const double initial_email_opening_probability = (*user_cfg.initial_email_opening_probability)(ctx.get_rng());
				const double email_opening_probability_multiplier_exponent = (*user_cfg.email_opening_probability_multiplier_exponent)(ctx.get_rng());
				const double probability_email_activated = (*user_cfg.probability_email_activated)(ctx.get_rng());
				const double probability_instant_email_check = (*user_cfg.probability_instant_email_check)(ctx.get_rng());
				User user(std::move(host_switcher), std::move(email_accounts), probability_email_activated, initial_email_opening_probability, email_opening_probability_multiplier_exponent, probability_instant_email_check);
				world.add_user(std::move(user));
				get_logger()->trace("SimulationConfigurator: Created a user.");
			}
			get_logger()->info("SimulationConfigurator: Created {} users of type {}", user_cfg.count, user_type_and_cfg.first);
		}
		get_logger()->info("SimulationConfigurator: Created {} laptops, {} of which have an antivirus program", nbr_laptops, nbr_laptops_with_antivirus);		
	}

	void SimulationConfigurator::generate_email_providers() {
		email_providers_.clear();
		for (const auto& kv : sim_config_.email_config.filtering_efficiency) {
			email_providers_[kv.first] = std::make_shared<EmailProvider>(kv.first, kv.second);
		}
		get_logger()->info("SimulationConfigurator: Created {} email providers.", email_providers_.size());
	}

	void SimulationConfigurator::generate_operating_system_vendors() {
		for (const auto& name_config : sim_config_.operating_system_vendors_config.vendors) {
			const auto& name = name_config.first;
			const auto& vendor_cfg = name_config.second;
			const auto vendor_ptr = std::make_shared<OperatingSystem::vendor_t>(name, vendor_cfg);
			for (const auto& spec_sched : vendor_cfg.release_schedules) {
				const auto spec = spec_sched.first;
				const auto it = os_vendor_cache_.find(spec);
				if (it != os_vendor_cache_.end()) {
					throw ConfigurationError("OperatingSystemVendor", (boost::locale::format("{1} and {2} are both vendors for operating system type {3} and version {4}")
						% vendor_ptr->get_name()
						% it->second->get_name()
						% spec.first
						% spec.second).str());
				}
				os_vendor_cache_[spec] = vendor_ptr;
			}
		}
	}

	void SimulationConfigurator::generate_antivirus_vendors() {
		for (const auto& name_config : sim_config_.antiviruses_config.vendors) {
			const auto& name = name_config.first;
			const auto& vendor_cfg = name_config.second;
			const auto vendor_ptr = std::make_shared<Antivirus::vendor_t>(name, vendor_cfg);
			for (const auto& spec_sched : vendor_cfg.release_schedules) {
				const auto spec = spec_sched.first;
				const auto it = av_vendor_cache_.find(spec);
				if (it != av_vendor_cache_.end()) {
					throw ConfigurationError("Antivirus", (boost::locale::format("{1} and {2} are both vendors for antivirus {3}")
						% vendor_ptr->get_name()
						% it->second->get_name()
						% spec).str());
				}
				av_vendor_cache_[spec] = vendor_ptr;
			}
		}
	}

	World SimulationConfigurator::create_world(Context& ctx) {
		World world;		
		os_vendor_cache_.clear();
		av_vendor_cache_.clear();
		corporate_local_networks_.clear();
		home_desktops_.clear();
		office_desktops_per_user_config_.clear();
		generate_operating_system_vendors();
		generate_antivirus_vendors();
		generate_local_networks(ctx, world);
		generate_email_providers();
		generate_users(ctx, world);
		world.count_things();
		describe_world(world);
		return world;
	}

	void SimulationConfigurator::describe_world(const World& world) const {
		if (!output_config_.network_structure_filename.empty()) {
			std::ofstream net_struct_file(output_config_.network_structure_filename, std::ios_base::trunc);
			world.save_network_structure(net_struct_file);
			net_struct_file.close();
			get_logger()->info("Saved network structure to file {}", output_config_.network_structure_filename);
		}
	}

	void SimulationConfigurator::validate_operating_system(const os_full_spec_t& os_full_spec, const Schedule& schedule) const {
		const auto os_spec = os_full_spec.first;
		for (const auto& name_config : sim_config_.operating_system_vendors_config.vendors) {
			const OperatingSystem::vendor_t::config_t& vendor_config = name_config.second;
			const auto it = vendor_config.release_schedules.find(os_spec);
			if (it != vendor_config.release_schedules.end()) {
				const int initial_latest_release = it->second.interpolate_and_extrapolate(schedule.get_start());
				if (os_full_spec.second > initial_latest_release) {
					throw ConfigurationError("OperatingSystem", (boost::locale::format("Operating system {1} version {2} has release number {3} larger than the latest release number {4} as of {5}")
						% os_spec.first
						% os_spec.second
						% os_full_spec.second
						% initial_latest_release
						% schedule.get_start()).str());
				}
			}
		}
	}
}
