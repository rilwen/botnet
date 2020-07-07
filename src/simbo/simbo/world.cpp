#include "enums.hpp"
#include "exceptions.hpp"
#include "laptop.hpp"
#include "world.hpp"
#include <unordered_map>
#include <unordered_set>

namespace simbo {
	World::World()
		: number_email_accounts_(0),
		number_hosts_(0),
		number_local_networks_(0)
	{
	}

	World::World(World&& other)
		: sysadmins_(std::move(other.sysadmins_)),
		users_(std::move(other.users_)),
		netadmins_(std::move(other.netadmins_)),
		isps_(std::move(other.isps_)),
		number_email_accounts_(other.number_email_accounts_),
		number_hosts_(other.number_hosts_),
		number_local_networks_(other.number_local_networks_)
	{
		other.wipe();
	}

	World& World::operator=(World&& other) {
		if (this != &other) {
			sysadmins_ = std::move(other.sysadmins_);
			users_ = std::move(other.users_);
			netadmins_ = std::move(other.netadmins_);
			isps_ = std::move(other.isps_);
			number_email_accounts_ = other.number_email_accounts_;
			number_hosts_ = other.number_hosts_;
			number_local_networks_ = other.number_local_networks_;
			other.wipe();
		}
		return *this;
	}

	void World::add_user(User&& user) {
		number_email_accounts_ += static_cast<int>(user.get_email_accounts().size());
		users_.push_back(std::move(user));
	}

	void World::add_sysadmin(Sysadmin&& sysadmin) {
		number_hosts_ += static_cast<int>(sysadmin.get_hosts().size());
		sysadmins_.push_back(std::move(sysadmin));
	}

	void World::add_network_admin(NetworkAdmin&& netadmin) {
		number_local_networks_ += static_cast<int>(netadmin.get_networks().size());
		netadmins_.push_back(std::move(netadmin));
	}

	void World::add_isp(InternetServiceProvider&& isp) {
		isps_.push_back(std::move(isp));
	}

	void World::count_things() {
		size_t new_number_hosts = 0;
		for (const auto& sysadmin : sysadmins_) {
			new_number_hosts += sysadmin.get_hosts().size();
		}
		number_hosts_ = static_cast<int>(new_number_hosts);
		size_t new_number_email_accounts = 0;
		for (const auto& user : users_) {
			new_number_email_accounts += user.get_email_accounts().size();
		}
		number_email_accounts_ = static_cast<int>(new_number_email_accounts);
		size_t new_number_local_networks = 0;
		for (const auto& network_admin : netadmins_) {
			new_number_local_networks += network_admin.get_networks().size();
		}
		number_local_networks_ = static_cast<int>(new_number_local_networks);
	}

	void World::save_network_structure(std::ostream& out) const {
		std::unordered_map<const LocalNetwork*, std::unordered_set<const User*>> desktop_users_per_network;
		std::unordered_map<const LocalNetwork*, std::unordered_set<const User*>> laptop_users_per_network;
		for (const auto& user : users_) {
			for (const auto* host : user.get_available_hosts()) {
				if (!host) {
					continue;
				}
				if (host->is_fixed()) {
					assert(host->get_type() == HostType::DESKTOP);
					// Jest tylko 1 sieć.
					const auto* ln = host->get_local_network();
					desktop_users_per_network[ln].insert(&user);
				} else {
					assert(host->get_type() == HostType::LAPTOP);
					// Zakładamy, że to Laptop.
					const auto* laptop = dynamic_cast<const Laptop*>(host);
					const auto laptop_networks(laptop->get_local_networks_view());
					const auto nbr_networks = laptop_networks.size();
					for (size_t i = 0; i < nbr_networks; ++i) {
						laptop_users_per_network[laptop_networks[i]].insert(&user);
					}
				}
			}
		}

		int network_admin_idx = 0;
		out << "network_admin_idx,local_network_id,local_network_type,country,nbr_desktops,nbr_laptops,nbr_servers,nbr_desktop_users,nbr_laptop_users,ip\n";
		for (const auto& network_admin : netadmins_) {
			for (const auto& ln : network_admin.get_networks()) {
				int nbr_desktops = 0;
				int nbr_servers = 0;
				int nbr_laptops = 0;
				for (const auto& host : ln->get_hosts()) {
					switch (host->get_type()) {
					case HostType::DESKTOP:
						++nbr_desktops;
						break;
					case HostType::LAPTOP:
						++nbr_laptops;
						break;
					case HostType::SERVER:
						++nbr_servers;
						break;
					default:
						throw std::invalid_argument((boost::locale::format("Unsupported host type {1} for host ID {2}") % to_string(host->get_type()) % host->get_id()).str());
					}
				}
				IpAddress ip_address;
				if (ln->get_public_connection()) {
					ip_address = ln->get_public_connection()->get_ip_address();
				}				
				out << network_admin_idx << "," << ln->get_id() << "," << ln->get_type() << "," << ln->get_country() << "," << nbr_desktops << "," << nbr_laptops << "," << nbr_servers << "," << desktop_users_per_network[ln.get()].size() << "," << laptop_users_per_network[ln.get()].size() << "," << ip_address.to_string() << "\n";
			}
			++network_admin_idx;
		}
	}

	void World::wipe() {
		sysadmins_.clear();
		users_.clear();
		netadmins_.clear();
		isps_.clear();
		number_email_accounts_ = 0;
		number_hosts_ = 0;
		number_local_networks_ = 0;
	}
}
