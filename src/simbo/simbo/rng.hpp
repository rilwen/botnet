#pragma once
#include <cstdint>
#include <limits>

namespace simbo {
	/// Abstrakcyjny interfejs do generatora liczb pseudolosowych. W celu kompatybilności z generatorami liczb losowych w STL, typ całkowity ma 64 bity.
	class RNG {
	public:
		typedef uint64_t int_type;

		/// Wirtualny destruktor.
		virtual ~RNG() {}

		/// Zwróć liczbę losową z rozkładem jednorodnym U(0, 1).
		virtual double draw_uniform() = 0;

		/// Zwróć liczbę losową z rozkładem Gaussa N(0, 1).
		virtual double draw_gaussian() = 0;
		
		/// Zwróć liczbę losową z rozkładem jednorodnym na przedziale [#get_int_min(), #get_int_max()].
		virtual int_type draw_uniform_integer() = 0;

		/// Przeskocz z liczb w generowanym ciągu losowym.
		virtual void discard(unsigned long long z) = 0;

		/// Minimalna wartość zwracana przez #draw_uniform_integer().
		static constexpr int_type get_int_min() {
			return 0u;
		}

		/// Maksymalna wartość zwracana przez #draw_uniform_integer().
		static constexpr int_type get_int_max() {
			return std::numeric_limits<int_type>::max();
		}

		/// Konwertuje implementację klasy RNG na generator liczb losowych spełniający wymagania STL.
		/// W ten sposób możemy korzystać z dowolnej implementacji RNG i algorytmów losowych STL.
		class StlView {
		public:
			typedef int_type result_type;
			StlView(RNG& rng)
				: _rng(rng) {}

			StlView(const StlView&) = default;

			StlView& operator=(const StlView&) = delete;

			static constexpr result_type min() {
				return get_int_min();
			}

			static constexpr result_type max() {
				return get_int_max();
			}

			void discard(unsigned long long z) {
				_rng.discard(z);
			}

			result_type operator()() {
				return _rng.draw_uniform_integer();
			}
		private:
			RNG& _rng;
		};

		/// Zwróć wersję generatora kompatybilną z STL. Uwaga: obiekt jest tymczasowy (zawiera referencję do *this).
		StlView make_stl() {
			return StlView(*this);
		}
	};
}
