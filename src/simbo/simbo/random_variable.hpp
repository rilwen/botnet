#pragma once

namespace simbo {
	class RNG;

	/**
	\brief Zmienna losowa.

	Szablon modeluje zmienną losową o określonej dystrybucji którą można próbkować podając jej generator liczb losowych.

	\tparam T Typ wartości zmiennej losowej.
	*/
	template <class T> class RandomVariable {
	public:
		/// Wirtualny destruktor.
		virtual ~RandomVariable() {

		}

		/// Próbkuj zmienną losową.
		virtual T operator()(RNG& rng) const = 0;
	};
}
