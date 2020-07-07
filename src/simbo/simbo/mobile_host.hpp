#pragma once
#include "host.hpp"

namespace simbo {
	class Context;

	/// Host mobilny.
	class MobileHost : public Host {
	public:
		const_local_network_ptr get_local_network() const override {
			return local_network_.get_value();
		}

		/// Zwróć czas ostatniej zmiany sieci lokalnej.
		seconds_t get_local_network_change_time() const {
			return local_network_.get_last_change_time();
		}

		void update_state(Context& ctx) override;

		bool is_fixed() const override {
			return false;
		}
	protected:
		/** \brief Konstruktor. Wołany z podklas.
		\param id Zobacz Host::Host
		\param operating_system Zobacz Host::Host
		\throws Zobacz Host::Host
		\throws std::domain_error Jeżeli local_network == nullptr.
		*/
		MobileHost(id_t id, operating_system_ptr&& operating_system);
	private:
		/**
		\brief Przyłącz hosta do sieci lokalnej.
		Jeżeli nowa sieć jest taka sama jak poprzednia, to wartość zwracana przez #get_local_network_change_time() nie ulegnie zmianie.
		\param local_network Wskaźnik do sieci lokalnej. Jeżeli jest null, odłącz hosta od aktualnej sieci.
		\param time Czas przyłączenia albo odłączenia.
		\throws std::domain_error Jeżeli time <= #get_local_network_change_time().
		*/
		void set_local_network(local_network_ptr local_network, seconds_t time);

		/** \brief Uaktualnij używaną sieć lokalną.
		Funkcja wybiera nową sieć ale jej nie ustawia.
		\param ctx Kontekst.
		\throws std::domain_error Jeżeli ctx.get_time() <= #get_local_network_change_time().
		\return Wskaźnik do nowej sieci. Może być nullptr.
		*/
		virtual local_network_ptr update_local_network(Context& ctx) = 0;

		/// Sieć do której należy komputer. Wskaźnik jest nie-const ponieważ musimy wyrejestrować hosta ze starej sieci kiedy przechodzimy do nowej.
		TrackedValue<local_network_ptr> local_network_;
	};
}
