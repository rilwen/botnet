#pragma once
#include <cmath>
#include <iosfwd>
#include <limits>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "json_fwd.hpp"

namespace simbo {
	/// Podstawowy typ do dat i czasu. Nie zawiera informacji o strefach czasowych (zakładamy UTC).
	typedef boost::posix_time::ptime DateTime;

	/// Typ opisujący wyłącznie daty.
	typedef boost::gregorian::date Date;

	/// Typ opisujący interwał czasowy składający się z godzin, minut, sekund i mikrosekund.
	typedef boost::posix_time::time_duration TimeDuration;

	/// Typ mierzący czas jako zmienną ciągłą (idealizacja matematyczna). Jednostką są sekundy.
	typedef double seconds_t;

	/// Minimalna data.
	inline DateTime min_datetime() {
		return DateTime(boost::posix_time::min_date_time);
	}

	/// Maksymalna data.
	inline DateTime max_datetime() {
		return DateTime(boost::posix_time::max_date_time);
	}

	/// Zwraca czas nieskończony.
	inline constexpr seconds_t infinite_time() {
		return std::numeric_limits<seconds_t>::infinity();
	}

	/// Zwraca czas nieokreślony.
	inline constexpr seconds_t undefined_time() {
		return std::numeric_limits<seconds_t>::quiet_NaN();
	}

	/// Sprawdź czy czas jest nieokreślony.
	inline bool is_time_undefined(seconds_t time) {
		return std::isnan(time);
	}

	/// Sprawdź czy czas jest nieokreślony.
	inline bool is_time_finite(seconds_t time) {
		return std::isfinite(time);
	}

	/// Konwertuj interwał czasowy do jego całkowitej długości w sekundach.
	inline seconds_t to_seconds(const TimeDuration& duration) {
		return static_cast<double>(duration.total_microseconds()) / 1e6;
	}

	/** \brief Konwertuj czas w sekundach do interwału czasowego.
	Zaokrągla w dół do całkowitej liczby mikrosekund.
	\param seconds Czas w sekundach.
	*/
	TimeDuration time_duration_from_seconds(seconds_t seconds);

	/// Sprawdź czy data to weekend (sobota albo niedziela).
	bool is_weekend(const Date& date);

	/// Konwertuj TimeDuration albo liczbę zmiennoprzecinkową do czasu w sekundach.
	seconds_t get_seconds_from_json(const json& j);

	/// Konwertuj łańcuch znaków do daty.
	inline Date from_string(const std::string& s) {
		return boost::gregorian::from_simple_string(s);
	}

	/// Konwertuj datę na tekst.
	inline std::string to_string(const Date& d) {
		return boost::gregorian::to_iso_extended_string(d);
	}

	/// Konwertuj datę-czas na tekst.
	inline std::string to_string(const DateTime& t) {
		return boost::posix_time::to_iso_extended_string(t);
	}
}

// Konwersje do/z JSON
namespace boost {
	namespace gregorian {
		/// Skonwertuj datę do formatu JSON.		
		void to_json(json& j, const boost::gregorian::date& d);

		/// \brief Odczytaj datę z formatu JSON.
		/// \param j Dane JSON: obiekt (z polami day, month i year) albo łańcuch znaków.
		void from_json(const json& j, boost::gregorian::date& d);
	}
	namespace posix_time {
		/// Skonwertuj interwał czasowy do formatu JSON.
		void to_json(json& j, const boost::posix_time::time_duration& td);

		/// \brief Odczytaj interwał czasowy z formatu JSON.
		/// Pola: hours, minutes, second i microseconds. Każde pole ma domyślną wartość 0 i może być pominięte.
		void from_json(const json& j, boost::posix_time::time_duration& td);

		/// Skonwertuj datę-czas do formatu JSON.
		void to_json(json& j, const boost::posix_time::ptime& ptime);

		/// \brief Odczytaj datę-czas z formatu JSON.
		/// \param j Dane JSON. Łańcuch znaków (interpretowany jako data bez pola "time") albo obiekt z polami date i time. Pole time jest nieobowiązkowe (domyślna wartość 0 godzin, 0 minut, 0 sekund, 0 mikrosekund).
		void from_json(const json& j, boost::posix_time::ptime& ptime);

		/// Drukuj datę-czas do strumienia wyjścia.
		std::ostream& operator<<(std::ostream& os, const boost::posix_time::ptime& ptime);
	}
}
