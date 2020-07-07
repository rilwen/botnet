#pragma once
#include <memory>
#include <unordered_set>
#include <utility>
#include "date_time.hpp"

namespace simbo {
	class Context;
	class EmailProvider;
	class Host;
	class JumpMarkovProcess;
	class LocalNetwork;
	class User;

	/// Konto pocztowe.
	class EmailAccount {
	public:
		typedef std::shared_ptr<const EmailProvider> email_provider_ptr;
		typedef const Host* host_ptr;
		typedef const LocalNetwork* local_network_ptr;
		typedef std::unordered_set<host_ptr> hosts_set;
		typedef std::unordered_set<local_network_ptr> local_networks_set;
		typedef std::shared_ptr<const JumpMarkovProcess> check_process_ptr;

		/** \brief Konstruktor
		\param email_provider Wskaźnik do dostawcy poczty.
		\param check_process Wskaźnik do procesu sterującego sprawdzaniem poczty: skok procesu z wartości 0 na 1 oznacza że użytkownik sprawdza pocztę.
		\param allowed_hosts Zbiór wskaźników do hostów na których dostępne jest to konto. Jeżeli jest pusty, oznacza to, że konto jest dostępne na dowolnym hoście.
		\param allowed_networks Zbiór wskaźników do sieci lokalnych w których dostępne jest to konto. Jeżeli jest pusty, oznacza to, że konto jest dostępne w dowolnej sieci.
		\throw std::invalid_argument Jeżeli email_provider albo check_process jest null, albo jeżeli jakiś wskaźnik w allowed_hosts jest null.
		*/
		EmailAccount(email_provider_ptr email_provider, check_process_ptr check_process, hosts_set&& allowed_hosts, local_networks_set&& allowed_networks);

		EmailAccount(EmailAccount&&) = default;

		EmailAccount(const EmailAccount&) = delete;
		EmailAccount& operator=(const EmailAccount&) = delete;

		/// Zwróć referencję do dostawcy poczty.
		const EmailProvider& get_email_provider() const {
			return *email_provider_;
		}

		/// Zwróć referencję do wektora hostów na których dostępne jest konto pocztowe (pusty wektor oznacza że konto jest dostępne na dowolnym hoście).
		const hosts_set& get_allowed_hosts() const {
			return allowed_hosts_;
		}

		/// Sprawdź czy host nadaje się do czytania poczty.
		/// \throw std::invalid_argument Jeżeli host == nullptr albo host->get_local_network() == nullptr.
		bool is_host_allowed(host_ptr host) const;

		/** \brief Sprawdź czy poczta będzie sprawdzona w następnym okresie symulacji.
		\param ctx Kontekst.
		\return Czas z przedziału [ctx.get_time(), ctx.get_next_time()) jeżeli poczta zostanie sprawdzona *po raz pierwszy* w tym przedziale, albo undefined_time() jeżeli nie będzie sprawdzona w tym przedziale w ogóle.
		\throw std::invalid_argument Jeżeli granice przedziału czasowego wykraczają poza granice na których określony jest check_process_.
		*/
		seconds_t email_check_time(Context& ctx) const;

		/**
		\brief Użytkownik dostaje do skrzynki pocztowej zainfekowany email.
		\param ctx Kontekst.
		\param time Czas otrzymania emaila.
		\throw std::domain_error Jeżeli !ctx.is_inside_current_time_step(time).
		*/
		void receive_infectious_email(Context& ctx, seconds_t time);

		/// Zwróć liczbę nieotwartych zainfekowanych emaili.
		int get_number_unopened_infectious_emails() const {
			return static_cast<int>(unopened_infectious_emails_.size());
		}

		/** \brief Zaznacz zainfekowany email jako otwarty, usuwając go z listy nieotwartych.
		\param email_index Indeks otwieranego emaila.
		\throw std::out_of_range Kiedy email_index < 0 albo email_index >= #get_number_unopened_infectious_emails().
		*/
		void open_infectious_email(int email_index);

		/** \brief Zostaw zainfekowany email nieotwarty.
		\param email_index Indeks otwieranego emaila.
		\throw std::out_of_range Kiedy email_index < 0 albo email_index >= #get_number_unopened_infectious_emails().
		*/
		void skip_infectious_email(int email_index);

		/** \brief Zwróć liczbę sesji kiedy email nie został otwarty.
		\throw std::out_of_range Kiedy email_index < 0 albo email_index >= #get_number_unopened_infectious_emails().
		*/
		int get_number_sessions_email_unopened(int email_index) const;

		/** \brief Ustaw monitorującego użytkownika.

		Ustaw wskaźnik do użytkownika którego trzeba powiadomić o przybyciu nowego emaila.

		\param user Wskaźnik. Podanie wartości nullptr oznacza wyłączenie monitoringu.
		*/
		void set_monitoring_user(User* user);

		/// Zwróć czas ostatniego sprawdzania konta.
		seconds_t get_last_check_time() const {
			return last_check_time_;
		}

		/** Ustaw czas ostatniego sprawdzania konta. Nie może być wcześniejszy niż poprzednia wartość #get_last_check_time().
		 * @param time Czas.
		 @throw std::domain_error Jeżeli time < #get_last_check_time().
		 */
		void set_last_check_time(seconds_t time);
	private:
		/// Wskaźnik do dostawcy poczty.
		email_provider_ptr email_provider_;

		/// Wskaźnik do procesu sterującego sprawdzaniem poczty: skok procesu oznacza że użytkownik sprawdza pocztę.
		check_process_ptr check_process_;

		/// Zbiór hostów na których dostępne jest konto pocztowe (pusty zbiór oznacza że konto jest dostępne na dowolnym hoście).
		hosts_set allowed_hosts_;

		/// Zbiór wskaźników do sieci lokalnych w których dostępne jest to konto. Jeżeli jest pusty, oznacza to, że konto jest dostępne w dowolnej sieci.
		local_networks_set allowed_networks_;

		/// Nieotwarte zainfekowane emaile (czas otrzymania, liczba otwarć skrzynki z pominięciem tego maila).
		std::vector<std::pair<int, seconds_t>> unopened_infectious_emails_;

		/// Wskaźnik do użytkownika śledzącego powiadomienia o nowych emailach.
		User* monitoring_user_;

		/// Czas ostatniego sprawdzania konta.
		seconds_t last_check_time_;

		bool is_network_allowed(local_network_ptr local_network) const;
	};
}
