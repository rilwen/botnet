#pragma once
#include <gtest/gtest_prod.h>
#include "context.hpp"
#include "data_storage_format.hpp"
#include "output_config.hpp"
#include "world.hpp"

namespace simbo {

	struct OutputConfig;

	/** \brief Steruje przebiegiem symulacji.
	*/
	class SimulationController {
	public:
		/** Konstruktor.
		\param world Referencja do obiektu World.
		\param ctx Referencja do kontekstu symulacji.
		\param output_config Referencja do konfiguracji zapisu wyników.
		\throw std::invalid_argument Jeżeli output_config.validate() zgłosi błąd, albo format zapisu wyników jest nieznany.
		\see DataStorageFormat, OutputConfig
		*/
		SimulationController(World&& world, Context&& ctx, const OutputConfig& output_config);

		/// Zrób jeden krok symulacji.
		void step();

		/** \brief Puść całą symulację i zapisz wyniki.
		Pliki zostaną nadpisane.
		*/
		void run();

		const World& get_world() const {
			return world_;
		}

		const Context& get_context() const {
			return ctx_;
		}
	private:
		World world_;
		Context ctx_;
		OutputConfig output_config_;
		DataStorageFormat data_storage_format_;

		/// \brief Wyślij emaile z exploitami.
		/// \return Liczba emaili wysłanych do kont pocztowych modelowanych w symulacji.
		int send_infectious_emails();

		void update_host_states();

		void update_user_states();

		void perform_user_actions();
	};
}
