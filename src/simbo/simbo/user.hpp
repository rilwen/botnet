#pragma once
#include <memory>
#include <vector>
#include "context.hpp"
#include "date_time.hpp"
#include "email_account.hpp"
#include "markov_switcher.hpp"

namespace simbo {

	class Context;
	class Host;

	/** \brief Użytkownik komputera.

	Użytkownik jest jednym z wektorów infekcji: klika na złe załączniki, ściąga malware, wchodzi na strony z niebezpiecznymi reklamami (np. Onet.pl).

	Klasa User implementuje następujący model interakcji z zainfekowanymi emailami:

	0. Sprawdzanie poczty jest inicjowane przez proces losowy albo przyjście nowego emaila.

	1. Za każdym razem, kiedy użytkownik otwiera skrzynkę pocztową, może otworzyć albo nie każdy ze znajdujących się w niej zainfekowanych emaili, każdy niezależnie.

	2. Jeżeli jest to już n-ty raz kiedy użytkownik widzi nieotwarty zainfekowany email, to prawdopodobieństwo jego otwarcia
	jest równe p_open = p_init_open / n^k.

	3. Jeżeli użytkownik otworzy zainfekowany email, to może go uaktywnić (włączyć załącznik) z prawdopodobieństwem p_activ.

	4. Jeżeli użytkownik uaktywni email, następuje próba infekcji.
	*/
	class User {
	public:
		typedef Host* host_ptr;
		typedef MarkovSwitcher<host_ptr> host_switcher_t;
		typedef std::vector<EmailAccount> email_accounts_vec;

		/** \brief Konstruktor.
		\param host_switcher Przełącza pomiędzy hostami.
		\param email_accounts Wektor kont pocztowych. Konta są przejmowane na własność przez użytkownika.
		\param probability_email_activated Prawdopodobieństwo, że użytkownik aktywuje zainfekowany email po otwarciu.
		\param initial_email_opening_probability Początkowe prawdopodobieństwo otwarcia zainfekowanego emaila.
		\param email_opening_probability_multiplier_exponent Potęga do której podnosimy dotychczasową liczbę otwarć skrzynki, żeby obliczyć współczynnik przez który dzielimy początkowe prawdopodobieństwo otwarcia zainfekowanego emaila.
		\param probability_instant_email_check Prawdopodobieństwo, że użytkownik sprawdzi email od razu po otrzymaniu go (jeżeli ma dostęp do komputera i sieci).
		\throw std::domain_error Jeżeli prawdopodobieństwa nie należą do przedziału [0, 1] albo wykładnik nie jest >= 0.
		*/
		User(host_switcher_t&& host_switcher, email_accounts_vec&& email_accounts, double probability_email_activated,
			double initial_email_opening_probability, double email_opening_probability_multiplier_exponent,
			double probability_instant_email_check);

		User(const User&) = delete;
		User& operator=(const User&) = delete;
		User& operator=(User&&) = delete;

		/// Konstruktor przenoszący.
		User(User&&);

		/** \brief Uaktualnij używanego hosta.
		\param ctx Kontekst.
		\throws std::domain_error Jeżeli ctx.get_time() nie jest większy od czasu poprzedniej aktualizacji.
		*/
		void update_used_host(Context& ctx);

		/// Zwróć czas ostatniej zmiany (nie aktualizacji) używanego hosta.
		seconds_t get_used_host_change_time() const {
			return host_switcher_.get_last_change_time();
		}

		/// Zwróć wskaźnik do używanego hosta (nullptr kiedy host nie jest używany).
		host_ptr get_used_host() {
			return host_switcher_.get_current_value();
		}

		/// Zwróć wskaźnik const do używanego hosta (nullptr kiedy host nie jest używany).
		const host_ptr get_used_host() const {
			return host_switcher_.get_current_value();
		}

		/// Symuluj sprawdzanie poczty w następnym przedziale czasowym symulacji [ctx.get_time(), ctx.get_next_time()).
		/// \return Liczba sprawdzonych kont.
		int check_email(Context& ctx);

		/// Zwróć referencję const do wektora kont pocztowych.
		const email_accounts_vec& get_email_accounts() const {
			return email_accounts_;
		}

		/// Zwróć iterator do początku wektora kont pocztowych.
		email_accounts_vec::iterator get_email_accounts_begin() {
			return email_accounts_.begin();
		}

		/// Zwróć iterator do końca wektora kont pocztowych.
		email_accounts_vec::iterator get_email_accounts_end() {
			return email_accounts_.end();
		}

		/// Zwróć referencję do i-tego konta pocztowego.
		EmailAccount& get_email_account(int i) {
			return email_accounts_[i];
		}

		/// Zwróć referencję const do i-tego konta pocztowego.
		const EmailAccount& get_email_account(int i) const {
			return email_accounts_[i];
		}

		/// Uaktualnij stan użytkownika.
		/// \param ctx Kontekst.
		void update_state(Context& ctx);

		/** \brief Wykonaj operacje w trakcie przedziału czasowego [ctx.get_time(), ctx.get_next_time()).
		 * \param ctx Kontekst.
		 */
		void perform_actions(Context& ctx);

		/// Zwróć stałą referencję do wektora wskaźników do hostów dostępnych użytkownikowi.
		const std::vector<host_ptr>& get_available_hosts() const {
			return host_switcher_.get_values();
		}

		/*! \brief Sprawdź email na koncie jeśli to możliwe.

		Użytkownik musi mieć dostęp do jakiegoś hosta.

		\param ctx Kontekst.
		\param email_account Konto pocztowe.
		\param check_time Czas sprawdzenia. Jeżeli równy undefined_time(), weź czas sprawdzenia z procesu losowego.
		\return Czy konto zostało sprawdzone.
		*/
		bool check_email_account(Context& ctx, EmailAccount& email_account, seconds_t check_time);

		/// Zwróć prawdopodobieństwo, że użytkownik sprawdzi email od razu po otrzymaniu go (jeżeli może).
		double get_probability_instant_email_check() const {
			return probability_instant_email_check_;
		}
	private:
		/// Przełącza pomiędzy hostami.
		host_switcher_t host_switcher_;

		/// Wektor kont pocztowych.
		email_accounts_vec email_accounts_;

		/// p_activ.
		double probability_email_activated_;

		/// p_init_open.
		double initial_email_opening_probability_;

		/// Potęga k.
		double email_opening_probability_multiplier_exponent_;

		/// Prawdopodobieństwo, że użytkownik sprawdzi email od razu po otrzymaniu go (jeżeli może).
		double probability_instant_email_check_;

		/// Ustaw prawdopodobieństwo, że użytkownik aktywuje zainfekowany email po otwarciu.
		/// \throw std::domain_error Jeżeli p nie należy do przedziału [0, 1].
		void set_probability_email_activated(double p);

		/// Ustaw początkowe prawdopodobieństwo otwarcia zainfekowanego emaila.
		/// \throw std::domain_error Jeżeli p nie należy do przedziału [0, 1].
		void set_initial_email_opening_probability(double p);

		/// Ustaw potęgę do której podnosimy dotychczasową liczbę otwarć skrzynki, żeby obliczyć współczynnik przez który dzielimy
		/// początkowe prawdopodobieństwo otwarcia zainfekowanego emaila.
		/// \throw std::domain_error Jeżeli k < 0
		void set_email_opening_probability_multiplier_exponent(double k);

		void set_email_account_monitoring();

		void wipe();

		bool has_access_to_internet() const;
	};
}
