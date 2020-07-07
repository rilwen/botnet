#pragma once
#include <memory>
#include <vector>
#include "host.hpp"
#include "markov_process_state.hpp"
#include "object_manager.hpp"

namespace simbo {

	class Context;
	class PausingJumpMarkovProcess;

	/**
	\brief Modeluje administratora systemu.

	Sysadmin zarządza hostami, administruje łaty bezpieczeństwa, itd.
	*/
	class Sysadmin: private ObjectManager<Host, Sysadmin> {
	public:
		// Skrócone nazwy typów.
		typedef object_ptr host_ptr;
		typedef objects_vec hosts_vec;

		/*! \brief Konstruktor.
		\param maintenance_trigger Proces który skacze kiedy administrator ma się zająć swoimi komputerami.
		\throw std::invalid_argument Jeżeli maintenance_trigger == nullptr.
		*/
		Sysadmin(std::shared_ptr<PausingJumpMarkovProcess> maintenance_trigger);

		/// Konstruktor przenoszący.
		Sysadmin(Sysadmin&& other) = default;

		virtual ~Sysadmin() {}
		
		/**
		\brief Dodaj nowy host.

		Funkcja przejmuje obiekt host na własność i ustawia this jako sysadmina dla host.
		
		\param host Wskaźnik do dodawanego hosta.

		\throws std::invalid_argument Jeżeli host == nullptr.
		*/
		void add_host(host_ptr&& host) {
			add_object(*this, std::move(host), Passkey<Sysadmin>());
		}

		/// Zwróć referencję do wektora hostów należących do sysadmina.
		const hosts_vec& get_hosts() const {
			return get_objects();
		}

		/// Wykonaj czynności administracyjne.
		/// \param ctx Kontekst.
		/// \throw std::domain_error Jeżeli ctx.get_time() nie jest późniejszy niż czas poprzedniego wywołania tej funkcji.
		void do_maintenance(Context& ctx);
	private:
		std::shared_ptr<PausingJumpMarkovProcess> maintenance_trigger_;
		MarkovProcessState<std::pair<int, seconds_t>> last_maintenance_;
	};
}
