#pragma once

namespace simbo {
	/**
	\brief Umożliwia klasie T dostęp do wybranych funkcji innej klasy.

	Sposób użycia: jeżeli klasa X chce dać klasie T, i tylko tej, dostęp do funkcji foo(double),
	powinna zadeklarować foo jako foo(double, Passkey<T>) i ignorować drugi argument.

	Klucz nie jest kopiowalny, więc funkcja która go otrzymała nie może użyć go do wywołania innej funkcji oczekującej
	tego klucza.

	\tparam T Klasa której udostępnia się dostęp.
	*/
	template <class T> class Passkey {
		friend T;
		Passkey() {

		}
		Passkey(const Passkey<T>&) = default;
	public:
		Passkey(Passkey<T>&&) {}
	};
}
