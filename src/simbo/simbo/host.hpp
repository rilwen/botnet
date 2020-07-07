#pragma once
#include <functional>
#include <memory>
#include "activity_state.hpp"
#include "antivirus.hpp"
#include "bot_state.hpp"
#include "date_time.hpp"
#include "host_type.hpp"
#include "id.hpp"
#include "infection_state.hpp"
#include "managed_object.hpp"
#include "operating_system.hpp"
#include "passkey.hpp"
#include "tracked_value_own_time.hpp"

namespace simbo {

	class Botnet;
	struct BotnetConfig;
	class Context;
	class LocalNetwork;
	template <class V> class MarkovProcess;
	template <class T> class Passkey;
	class Sysadmin;
	
	/**
	\brief Interfejs reprezentujący host (komputer / urządzenie IoT podłączone na stałe albo okazjonalnie do sieci).

	Host jest infekowany i leczony, w stanie zainfekowanym wysyła pakiety, jest aktywny albo nie, itd. Obiekt Host jest zarządzany
	i należy do obiektu klasy Sysadmin.
	*/
	class Host : public ManagedObject<Host, Sysadmin> {
	public:
		// Skrócone nazwy typów.		
		typedef std::unique_ptr<OperatingSystem> operating_system_ptr;
		typedef Id::id_t id_t;
		typedef LocalNetwork* local_network_ptr;
		typedef const LocalNetwork* const_local_network_ptr;

		/// Wirtualny destruktor.
		virtual ~Host() {}

		// Host nie jest kopiowalny.
		Host(const Host&) = delete;
		Host& operator=(const Host&) = delete;

		/// \brief Zwróć identyfikator hosta.
		id_t get_id() const {
			return id_;
		}

		/// \brief Zwróć wskaźnik do aktualnej sieci lokalnej w której znajduje się host. 
		/// Zwracana wartość może być nullptr.
		virtual const_local_network_ptr get_local_network() const = 0;

		/// \brief Zwróć referencję do sysadmina.
		const Sysadmin& get_sysadmin() const {
			return get_manager();
		}

		/// Sprawdź czy host ma dostęp do internetu.
		bool has_internet_access() const;

		/// Sprawdź czy host jest włączony.
		bool is_on() const {
			return get_activity_state().get_value() == ActivityState::ON;
		}

		/// \brief Zwróć stan aktywności hosta.
		const TrackedValue<ActivityState>& get_activity_state() const {
			return activity_state_;
		}

		/// \brief Zwróć rolę hosta w botnecie.
		const TrackedValueOwnTime<BotState>& get_bot_state() const {
			return bot_state_;
		}

		/// \brief Włącz hosta, chyba że #is_always_on() == true.
		/// \param ctx Kontekst symulacji.
		/// \param time Czas włączenia.
		/// \throws std::domain_error Jeżeli time <= #get_activity_state().get_last_update_time() albo jeżeli !ctx.is_inside_current_time_step(time).
		void turn_on(Context& ctx, seconds_t time);

		/// \brief Wyłącz hosta, chyba że #is_always_on() == true.
		/// \param ctx Kontekst symulacji.
		/// \param time Czas wyłączenia.
		/// \throws std::domain_error Jeżeli ctx.get_time() <= #get_activity_state().get_last_update_time() albo jeżeli !ctx.is_inside_current_time_step(time).
		void turn_off(Context& ctx, seconds_t time);

		/// \brief Zwróć stan infekcji hosta.
		const TrackedValue<InfectionState>& get_infection_state() const {
			return infection_state_;
		}

		/**
		\brief Spróbuj zainfekować hosta poprzez email.
		Host musi być włączony (powinno być sprawdzane przez kod wołający tę funkcję).
		\param ctx Kontekst.
		\param time Czas infekcji.
		\throws std::logic_error Jeżeli host nie jest w stanie "nie zainfekowany".
		\throws std::domain_error Jeżeli time < #get_infection_state().get_last_change_time() albo jeżeli !ctx.is_inside_current_time_step(time).
		*/
		void try_infecting_via_email(Context& ctx, seconds_t time);

		/**
		\brief Zainfekuj hosta.
		Host musi być włączony.
		\param ctx Kontekst.
		\param time Czas infekcji.
		\throws std::logic_error Jeżeli host nie jest w stanie "nie zainfekowany".
		\throws std::domain_error Jeżeli time < #get_infection_state().get_last_change_time() albo jeżeli time nie jest w przedziale [ctx.get_time(), ctx.get_next_time()).
		*/
		void infect(Context& ctx, seconds_t time);

		/**
		\brief Wylecz hosta.
		\param ctx Kontekst.
		\throws std::logic_error Jeżeli host nie jest w stanie "zainfekowany".
		\throws std::domain_error Jeżelictx.get_time() < #get_infection_state().get_last_change_time().
		*/
		void cure(Context& ctx);

		/** \brief Zresetuj hosta do ustawień fabrycznych / swieżej instalacji systemu.
		Usuwa infekcję ale nie leczy hosta.
		\param ctx Kontekst.
		\throws std::domain_error Jeżeli ctx.get_time() < #get_infection_state().get_last_change_time().
		*/
		void reset(Context& ctx);

		/// \brief Zwróć referencję do systemu operacyjnego hosta.
		const OperatingSystem& get_operating_system() const {
			return *os_;
		}

		/// Uaktualnij system operacyjny.
		/// Host powinien być włączony i mieć dostęp do sieci.
		void update_operating_system_release(Context& ctx);

		/// \brief Uaktualnij stan hosta.
		/// \param ctx Kontekst.
		/// \throw std::domain_error Jeżeli ctx.get_time() nie jest późniejszy niż czas poprzedniej aktualizacji.
		virtual void update_state(Context& ctx);

		/// Czy host ma przypisaną rolę w botnecie.
		bool has_botnet_role() const {
			return get_bot_state().get_value() != BotState::NONE;
		}

		/// Czy host jest zawsze włączony, czy tylko wtedy kiedy jest używany przez użytkownika?
		bool is_always_on() const {
			return always_on_;
		}

		/// Ustaw czy host jest zawsze włączony.
		void set_is_always_on(bool always_on) {
			always_on_ = always_on;
		}

		/// Zwróć liczbę użytkowników korzystających z hosta.
		int get_user_counter() const {
			return user_counter_;
		}

		/** \brief Usuń użytkownika korzystającego z hosta.

		\throw std::domain_error Jeżeli #get_user_counter() <= 0.
		*/
		void remove_user() {
			if (user_counter_ > 0) {
				--user_counter_;
			} else {
				throw std::logic_error(boost::locale::translate("Removing user when no users using the host"));
			}
		}

		/// Dodaj kolejnego użytkownika korzystającego z hosta.
		void add_user() {
			++user_counter_;
		}

		/// Host stały czy mobilny.
		virtual bool is_fixed() const = 0;

		/// Ustaw program antywirusowy.
		void set_antivirus(std::unique_ptr<Antivirus>&& av) {
			av_ = std::move(av);
		}

		/// Czy host ma zainstalowany program antywirusowy.
		bool has_antivirus() const {
			return av_ != nullptr;
		}

		/// Zwróć typ hosta.
		virtual HostType get_type() const = 0;

		/// Zwróć numer edycji kodu botneta który zainfekował hosta.
		/// \throw std::logic_error Jeżeli #has_botnet_role() == false.
		int get_payload_release_number() const;

		/// Zwróć jak długo (w sumie) host był włączony po ostatnim pingu do botnetu.
		const OwnTimeCounter& get_uptime_since_last_ping() const {
			return uptime_since_last_ping_;
		}

		/// Zwróć jak długo (w sumie) host był włączony i online po dołączeniu do botnetu.
		const OwnTimeCounter& get_online_time_since_joining_botnet() const {
			return online_time_since_joining_botnet_;
		}

		/// Zwróć czas kiedy ostatni raz host wysłał wiadomość do botnetu.
		seconds_t get_last_ping_time() const {
			return last_ping_time_;
		}
	protected:
		/**
		Konstruktor.

		Inicjalizuje hosta, ale nie ustawia jego właściciela.

		\param id Identyfikator hosta.
		\param operating_system Wskaźnik do systemu operacyjnego hosta.
		\param always_on Czy host jest zawsze włączony.
		
		\throws std::invalid_argument Jeżeli operating_system == nullptr albo activity_process == nullptr;
		\throws std::domain_error Jeżeli Id::is_good(id) == false.
		*/
		Host(id_t id, operating_system_ptr&& operating_system, bool always_on);

		/// Uaktualnij pomiary jak długo host był włączony od pewnych zdarzeń.
		void update_own_times(seconds_t wall_time);
	private:
		/// Identyfikator hosta.
		id_t id_;
		
		/// System operacyjny hosta.
		operating_system_ptr os_;

		/// Czy host jest zawsze włączony.
		bool always_on_;

		/// Stan aktywności hosta.
		TrackedValue<ActivityState> activity_state_;

		/// Stan infekcji hosta.
		TrackedValue<InfectionState> infection_state_;

		/// Edycja kodu botneta który zainfekował hosta.
		int payload_release_number_;

		/// Rola hosta w botnecie. Śledzi ile czasu "uptime" (kiedy host jest włączony) minęło od ostatniej zmiany roli.
		TrackedValueOwnTime<BotState> bot_state_;

		/// Czas ostatniego pingu do botnetu.
		seconds_t last_ping_time_;

		/// Ile czasu host był włączony w okresie po ostatnim pingu do botnetu.
		OwnTimeCounter uptime_since_last_ping_;

		/// Ile czasu host był online po dołączeniu do botnetu.
		OwnTimeCounter online_time_since_joining_botnet_;

		/// Liczba użytkowników korzystających z hosta.
		int user_counter_;

		/// Antywirus.
		std::unique_ptr<Antivirus> av_;

		/// Czas ostatniego skanu antywirusem.
		seconds_t av_last_scan_time_;

		/// Zmień stan zainfekowania. Host musi być włączony.
		void transition_infection_state(InfectionState new_state, InfectionState required_present_state, seconds_t time);

		void set_activity_state(Context& ctx, seconds_t time, ActivityState new_state);

		/// Uaktualnij stan bota. Host musi być włączony, zainfekowany, być częścią botnetu i mieć dostęp do sieci.
		/// \param ctx Kontekst symulacji.
		/// \param time Czas uaktualnienia.
		/// \throws std::domain_error Jeżeli time nie jest w przedziale[ctx.get_time(), ctx.get_next_time()).
		void update_bot_state(Context& ctx, seconds_t time);

		/** \brief Ustaw nową rolę hosta w botnecie.
		\param new_state Nowy stan bota.
		\param time Czas zmiany.
		\throws Zobacz również TrackedValue<BotState>#update.
		*/
		void set_bot_state(BotState new_state, seconds_t time);		

		/// Sprawdź czy bot wymaga rekonfiguracji. Host musi być włączony, zainfekowany, być częścią botnetu i mieć dostęp do sieci.
		bool bot_needs_reconfiguration(const BotnetConfig& botnet_config, const seconds_t time) const;
		
		/// Sprawdź czy bot musi pingnąć centrum dowodzenia. Host musi być włączony, zainfekowany, być częścią botnetu i mieć dostęp do sieci.
		/// \return Wektor czasów w których pingi mają być wysłane (<= time).
		std::vector<seconds_t> get_new_pings_times(const BotnetConfig& botnet_config, const seconds_t time) const;

		/// Uaktualnij zarchiwizowany czas ostatniego pingu do centrum dowodzenia botnetu.
		void update_ping_time(seconds_t wall_time);

		/// Dołącz do botnetu. Host musi być włączony, zainfekowany i mieć dostęp do sieci.
		void join_botnet(Context& ctx, seconds_t time);

		/// Opuść botnet. Host musi być włączony i nie być zainfekowany.
		void leave_botnet(Context& ctx);		

		/// Skanuj hosta antywirusem. Host musi być włączony i mieć zainstalowany antywirus.
		void perform_antivirus_scan(Context& ctx);

		/// Poproś botnet o nową konfigurację.
		void reconfigure_bot(Context& ctx, seconds_t time);

		/// Wyślij wiadomość do botnetu.
		void ping_botnet(Context& ctx, seconds_t time);
	};
}
