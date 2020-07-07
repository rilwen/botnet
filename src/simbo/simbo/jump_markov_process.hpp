#pragma once
#include <vector>
#include "markov_process.hpp"

namespace simbo {
	/// Proces który skacze.
	class JumpMarkovProcess : public MarkovProcess<int> {
	public:
		/** \brief Zwraca czas następnego skoku w ramach podanego horyzontu.

		\param rng Generator liczb losowych.
		\param previous_count Wyjściowa liczba skoków.
		\param previous_time Wyjściowy czas.
		\param horizon Maksymalny czas do którego zliczamy skoki.

		\return Jeżeli następny krok zachodzi w czasie t < horizon, zwraca t. W przeciwnym wypadku zwraca wartość horizon.

		\throw std::domain_error Jeżeli previous_time <= horizon, previous_time < #get_first_time() albo horizon > #get_last_time(). Jeżeli previous_count < #get_min_state().
		*/
		virtual seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const = 0;

		/**
		\brief Wygeneruj skoki w okresie [previous_time, next_time).
		\param rng Generator liczb losowych.
		\param previous_state Zobacz #get_next_state
		\param previous_time Zobacz #get_next_state
		\param next_time Zobacz #get_next_state
		\param jump_times Referencja do wektora do którego będą dodane czasy, poprzez wywoływanie jump_times.push_back(czas).
		\throw Zobacz #get_next_state.
		*/
		void generate_jumps(RNG& rng, const int& previous_state, seconds_t previous_time, seconds_t next_time, std::vector<seconds_t>& jump_times) const;

		/// Zwraca wartość previous_state powiększoną o liczbę wydarzeń z okresu [previous_time, next_time).
		int get_next_state(RNG& rng, const int& previous_state, seconds_t previous_time, seconds_t next_time) const override;

		/// Zwraca wartość max(other_process_state, #get_min_state()).
		int instant_switch(RNG& rng, const int& other_process_state, seconds_t time) const override;

		/// Zwraca minimalną wartość stanu procesu.
		virtual int get_min_state() const = 0;
	};
}
