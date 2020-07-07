#pragma once
#include <memory>
#include <utility>
#include "json_fwd.hpp"
#include "jump_markov_process.hpp"
#include "markov_process.hpp"

namespace simbo {

	class Schedule;

	/// \brief Proces który po skoku zatrzymuje się na ustalony czas.
	/// Stan procesu to para (wartość, czas ostatniego skoku).
	class PausingJumpMarkovProcess : public MarkovProcess<std::pair<int, seconds_t>> {
	public:
		typedef std::unique_ptr<const JumpMarkovProcess> base_jump_process_ptr;

		/** \brief Konstruktor.
		\param base_jump_process Proces który skacze bez pauzy. Konstruowany proces po każdym skoku "zamraża" się na ustalony czas a następnie ewoluuje zgodnie z base_jump_process.
		\param pause_length Okres pauzy.
		\throw std::invalid_argument Jeżeli base_jump_process == nullptr.
		\throw std::domain_error Jeżeli pause_length < 0.
		*/
		PausingJumpMarkovProcess(base_jump_process_ptr&& base_jump_process, seconds_t pause_length);

		/** \brief Konstruktor tworzący pauzowany jednorodny proces Poissona
		\param lambda Intensywność procesu Poissona.
		\param pause_length Okres pauzy.
		\throw std::domain_error Jeżeli pause_length < 0 albo lambda < 0.
		*/
		PausingJumpMarkovProcess(double lambda, seconds_t pause_length);

		state_type get_next_state(RNG& rng, const state_type& previous_state, seconds_t previous_time, seconds_t next_time) const override;

		state_type instant_switch(RNG& rng, const state_type& other_process_state, seconds_t time) const override;

		seconds_t get_first_time() const override {
			return base_jump_process_->get_first_time();
		}

		seconds_t get_last_time() const override {
			return base_jump_process_->get_last_time();
		}

		/// Zwróć długość pauzy po skoku (w sekundach).
		seconds_t get_pause_length() const {
			return pause_length_;
		}
	private:
		/// Proces generujacy skoki "niezatrzymane".
		base_jump_process_ptr base_jump_process_;

		/// Okres na jaki proces jest zatrzymywany po każdym skoku.
		seconds_t pause_length_;
	};

	/** \brief Odczytaj PausingJumpMarkovProcess z formatu JSON.
	\param j Dane JSON. Wymagane pola: "base_process", "pause_length".
	\param schedule Harmonogram
	\param process Referencja do wskaźnika do tworzonego procesu. Po udanym wywołaniu funkcji process != nullptr.
	\throw DeserialisationError W razie problemów z deserializacją.
	*/
	void from_json(const json& j, const Schedule& schedule, std::unique_ptr<PausingJumpMarkovProcess>& process);
}
