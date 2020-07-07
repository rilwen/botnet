#pragma once
#include <memory>
#include <unordered_set>
#include <vector>
#include "date_time.hpp"
#include "id.hpp"
#include "local_network_type.hpp"
#include "managed_object.hpp"
#include "passkey.hpp"

namespace simbo {

	class Host;
	class FixedHost;
	class MobileHost;
	class NetworkAdmin;
	class PublicConnection;

	/**
	\brief Abstrakcyjna klasa reprezentujaca lokalną sieć komputerową.
	*/
	class LocalNetwork: public ManagedObject<LocalNetwork, NetworkAdmin> {
	public:
		// Skrócone nazwy typów.
		typedef std::unordered_set<const Host*> host_set;
		typedef Id::id_t id_t;
		typedef const PublicConnection* public_connection_ptr;		

		/**
		\brief Konstruktor.
		\param typ Typ sieci lokalnej.
		\param country Kod kraju do którego należy sieć, dodatni.
		\throw std::domain_error Jeżeli country <= 0.
		*/
		LocalNetwork(id_t id, LocalNetworkType typ, int country);

		/// Wirtualny destruktor.
		virtual ~LocalNetwork() {}

		/// \brief Zwróć identyfikator sieci.
		id_t get_id() const {
			return id_;
		}

		/// Zwróć typ sieci.
		LocalNetworkType get_type() const {
			return type_;
		}

		/// Zwróć referencję do sysadmina.
		const NetworkAdmin& get_network_admin() const {
			return get_manager();
		}

		/** \brief Podłącz sieć do internetu (jeżeli public_connection nie jest null) albo odłącz (jeśli jest).

		\param public_connection Wskaźnik do publicznego łącza.
		*/
		void set_public_connection(public_connection_ptr public_connection) {
			public_connection_ = public_connection;
		}

		/// Zwróć wskaźnik do publicznego łącza internetowego. Nullptr jeżeli sieć jest odcięta od Internetu.
		public_connection_ptr get_public_connection() const {
			return public_connection_;
		}

		/**
		\brief Dodaj hosta do sieci.
		\param host Wskaźnik do hosta (nie przejmowany na własność).
		\param time Czas operacji.
		\param key Ignorowany (kontrola dostępu).
		\throw std::invalid_argument Jeżeli host->get_local_network() != this.
		*/
		void add_host(const Host* host, seconds_t time, Passkey<FixedHost> key) {
			add_host(host, time);
		}

		/**
		\see #add_host(const Host*, Passkey<FixedHost>)
		*/
		void add_host(const Host* host, seconds_t time, Passkey<MobileHost> key) {
			add_host(host, time);
		}

		/**
		\brief Usuń hosta z sieci.
		\param host Wskaźnik do hosta (nie przejmowany na własność).
		\param time Czas operacji.
		\param key Ignorowany (kontrola dostępu).
		\throw std::invalid_argument Jeżeli host->get_local_network() != this.
		*/
		void remove_host(const Host* host, seconds_t time, Passkey<FixedHost> key) {
			remove_host(host, time);
		}

		/**
		\see #remove_host(const Host*, Passkey<FixedHost>)
		*/
		void remove_host(const Host* host, seconds_t time, Passkey<MobileHost> key) {
			remove_host(host, time);
		}

		/// Zwróć referencję const do zbioru hostów podłączonych do tej sieci.
		const host_set& get_hosts() const {
			return hosts_;
		}

		/// Zwróć kod kraju do którego należy sieć.
		int get_country() const {
			return country_;
		}
	private:
		/// Identyfikator.
		id_t id_;

		/// Typ sieci.
		LocalNetworkType type_;

		/// Adres publiczny sieci.
		public_connection_ptr public_connection_;

		/// Hosty należące do tej sieci.
		host_set hosts_;

		/// Kod kraju do którego należy sieć.
		int country_;

		void add_host(const Host* host, seconds_t time);

		void remove_host(const Host* host, seconds_t time);
	};
}
