#pragma once
#include <iosfwd>
#include "internet_service_provider.hpp"
#include "network_admin.hpp"
#include "sysadmin.hpp"
#include "user.hpp"

namespace simbo {
	/** \brief Świat w którym dzieje się symulacja.

	Klasa zawiera w sobie wszystkie obiekty używane przez symulację.
	*/
	class World {
	public:
		/// Stwórz pusty świat.
		World();

		/// Konstruktor przenoszący.
		/// \param other Inny obiekt World.
		World(World&& other);

		/// Domyślny przenoszący operator przypisania.
		/// \param other Inny obiekt World.
		/// \return Referencja do obiektu po przypisaniu.
		World& operator=(World&& other);

		const std::vector<Sysadmin>& get_sysadmins() const {
			return sysadmins_;
		}

		const std::vector<User>& get_users() const {
			return users_;
		}

		const std::vector<NetworkAdmin>& get_network_admins() const {
			return netadmins_;
		}

		const std::vector<InternetServiceProvider>& get_isps() const {
			return isps_;
		}

		/** \brief Dodaj nowego użytkownika.
		\param user Użytkownik.
		*/
		void add_user(User&& user);

		/// Dodaj nowego sysadmina.
		void add_sysadmin(Sysadmin&& sysadmin);

		/// Dodaj nowego administratora sieci.
		void add_network_admin(NetworkAdmin&& netadmin);

		/// Dodaj nowego ISP.
		void add_isp(InternetServiceProvider&& isp);

		/// Zwróć liczbę kont email.
		int get_number_email_accounts() const {
			return number_email_accounts_;
		}

		/// Zwróć liczbę hostów. 
		int get_number_hosts() const {
			return number_hosts_;
		}

		/// Zwróć liczbę sieci lokalnych.
		int get_number_local_networks() const {
			return number_local_networks_;
		}

		/// Zwróć iterator do początku wektora użytkowników.
		std::vector<User>::iterator get_users_begin() {
			return users_.begin();
		}

		/// Zwróć iterator do końca wektora użytkowników.
		std::vector<User>::iterator get_users_end() {
			return users_.end();
		}

		/// Zwróć iterator do początku wektora sysadminów.
		std::vector<Sysadmin>::iterator get_sysadmins_begin() {
			return sysadmins_.begin();
		}

		/// Zwróć iterator do końca wektora sysadminów.
		std::vector<Sysadmin>::iterator get_sysadmins_end() {
			return sysadmins_.end();
		}

		/// Zwróć iterator do początku wektora administratorów/ek sieciowych.
		std::vector<NetworkAdmin>::iterator get_network_admins_begin() {
			return netadmins_.begin();
		}

		/// Zwróć iterator do końca wektora administratorów/ek sieciowych.
		std::vector<NetworkAdmin>::iterator get_network_admins_end() {
			return netadmins_.end();
		}

		/// Zwróć iterator do początku wektora ISP-ów.
		std::vector<InternetServiceProvider>::iterator get_isps_begin() {
			return isps_.begin();
		}

		/// Zwróć iterator do końca wektora ISP-ów.
		std::vector<InternetServiceProvider>::iterator get_isps_end() {
			return isps_.end();
		}

		/// Uaktualnij przechowywane sumy.
		void count_things();		

		/// Zapisz strukturę sieci do strumienia wyjściowego, w formacie CSV.
		void save_network_structure(std::ostream& out) const;
	private:
		std::vector<Sysadmin> sysadmins_;

		std::vector<User> users_;

		std::vector<NetworkAdmin> netadmins_;

		std::vector<InternetServiceProvider> isps_;

		int number_email_accounts_;

		int number_hosts_;

		int number_local_networks_;		

		/// Wyczyść obiekt po przeniesieniu.
		void wipe();
	};
}
