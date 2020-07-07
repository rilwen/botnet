#pragma once
#include <memory>
#include "json_fwd.hpp"
#include "jump_markov_process.hpp"

namespace simbo {

	/**
	\brief Process Poissona o stałej intensywności.

	Stan procesu to liczba całkowita, zwiększana o 1 kiedy proces zarejestruje wydarzenie.
	*/
	class SimplePoissonProcess : public JumpMarkovProcess {
	public:
		/**
		\brief Konstruktor.
		\param lambda Stała intensywność.
		\throws std::domain_error Jeżeli lambda < 0
		*/
		SimplePoissonProcess(double lambda);

		seconds_t get_first_time() const override {
			return -infinite_time();
		}

		seconds_t get_last_time() const override {
			return infinite_time();
		}

		seconds_t get_next_jump(RNG& rng, int previous_count, seconds_t previous_time, seconds_t horizon) const override;

		int get_min_state() const override {
			return std::numeric_limits<int>::min();
		}

		/// Zwróć wartość intensywności.
		/// \return Nieujemna liczba.
		double get_lambda() const {
			return lambda_;
		}
	private:
		/// Intensywność.
		double lambda_;
	};

	/** \brief Odczytaj SimplePoissonProcess z formatu JSON.
	Wymagane pola: "lambda" albo "inverse_lambda" (ale nie oba na raz).
	\param j Dane JSON.
	\param process Referencja do wskaźnika do tworzonego procesu. Po udanym wywołaniu funkcji process != nullptr.
	\throw DeserialisationError W razie problemów z deserializacją.
	*/
	void from_json(const json& j, std::unique_ptr<SimplePoissonProcess>& process);
}
