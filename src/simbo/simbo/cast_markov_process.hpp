#pragma once
#include "exceptions.hpp"
#include "markov_process.hpp"

namespace simbo {
	/// Proces generujący stany typu Visible zaimplementowany poprzez proces typu Hidden i statyczną konwersję typów.
	/// \tparam Visible Typ widocznych wartości.
	/// \tparam Hidden Typ "ukrytych" wartości.
	template <class Visible, class Hidden> class CastMarkovProcess : public MarkovProcess<Visible> {
	public:
		typedef std::unique_ptr<MarkovProcess<Hidden>> hidden_markov_process_ptr;

		/// Konstruktor.
		/// \throw std::invalid_argument Jeżeli hidden jest null.
		CastMarkovProcess(hidden_markov_process_ptr&& hidden)
			: hidden_(std::move(hidden)) {
			if (!hidden_) {
				throw std::invalid_argument(boost::locale::translate("Hidden process is null"));
			}
		}

		seconds_t get_first_time() const override {
			return hidden_->get_first_time();
		}

		seconds_t get_last_time() const override {
			return hidden_->get_last_time();
		}

		state_type get_next_state(RNG& rng, const state_type& previous_state, seconds_t previous_time, seconds_t next_time) const override {
			return static_cast<Visible>(hidden_->get_next_state(rng, static_cast<Hidden>(previous_state), previous_time, next_time));
		}

		state_type instant_switch(RNG& rng, const state_type& other_process_state, seconds_t time) const override {
			return static_cast<Visible>(hidden_->instant_switch(rng, static_cast<Hidden>(other_process_state), time));
		}
	private:
		hidden_markov_process_ptr hidden_;
	};
}
