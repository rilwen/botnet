#pragma once
#include <cassert>
#include <vector>
#include "date_time.hpp"
#include "json_fwd.hpp"

namespace simbo {

	/**
	\brief Plan czasowy symulacji.

	Klasa Schedule zawiera listę punktów czasowych symulacji i dostarcza informacji o ich datach i interwałach czasowych (w sekundach) pomiędzy nimi.
	*/
	class Schedule {
	public:
		/// Konstruuje pojedynczy punkt.
		Schedule(DateTime start);

		/**
		\brief Konstruuje punkty w równych odstępach czasowych.

		\param start Pierwszy punkt czasowy.
		\param period Długość odstępu czasowego pomiędzy punktami.
		\param nbr_points Liczba punktów czasowych, > 0.
		\throws std::invalid_argument Jeżeli step_size <= 0.
		\throws std::domain_error Jeżeli nbr_points <= 0.
		*/
		Schedule(DateTime start, TimeDuration period, int nbr_points);

		/** \brief Konstruuje plan czasowy z listy rosnących interwałów czasowych.
		Utworzony obiekt będzie miał jako pierwszy punkt czasowy 1970-01-01 00:00 oraz wartości #get_interval_from_start() zgodne z intervals_from_start. Jeżeli wektor intervals_from_start nie zawiera wartości zero, będzie ona dodana automatycznie.
		\param intervals_from_start Wektor interwałów czasowych (w sekundach) o wartościach ściśle rosnących i nieujemnych. 
		\throw std::invalid_argument Jeżeli interwały nie są ściśle rosnące.
		*/
		Schedule(std::vector<seconds_t>&& intervals_from_start);

		/** \brief Konstruuje plan czasowy z listy rosnących punktów czasowych.
		\param points Niepusty wektor punktów czasowych o ściśle rosnących wartościach.
		\throw std::invalid_argument Jeżeli points.empty() albo jeżeli !(points[i] > points[i - 1]).
		*/
		Schedule(std::vector<DateTime>&& points);

		/** \brief Konstruuje plan czasowy z punktami rozmieszczonymi zależnie od dnia tygodnia.
		\param start_date Data początkowa.
		\param workday_offsets Przesunięcia punktów czasowych (od czasu 00:00) w dni robocze.
		\param weekend_offsets Przesunięcia punktów czasowych (od czasu 00:00) w weekend.
		\param nbr_days Liczba dni.
		\param complete_last_day Czy na końcu ostatniego dnia dodawać przesunięcie 24h, tzn. punkt czasowy następnego dnia o 00:00.
		\throw std::invalid_argument Jeżeli przesunięcia nie są ściśle rosnące.
		\throw std::domain_error Jeżeli nbr_days <= 0, albo jeżeli któreś przesunięcie nie należy do przedziału [0, 24h).
		*/
		Schedule(Date start_date, const std::vector<TimeDuration>& workday_offsets, const std::vector<TimeDuration>& weekend_offsets, int nbr_days, bool complete_last_day);

		/// Konstruktor przenoszący.
		/// \param other Inny plan.
		Schedule(Schedule&& other);

		/// Przenoszący operator przypisania.
		/// \param other Inny plan.
		/// \return Referencja do planu po przypisaniu.
		Schedule& operator=(Schedule&& other);

		/// Zwróć liczbę punktów czasowych.
		int size() const {
			return size_;
		}

		/**
		\brief Zwróc i-ty punkt czasowy.
		\param i Indeks, 0 <= i < #size().
		*/
		DateTime get_point(int i) const {
			assert(i >= 0);
			assert(i < size());
			return points_[i];
		}

		/** \brief Zwróć interwał czasowy w sekundach pomiędzy i1-tym a i2-tym punktem.
		\param i1 Indeks pierwszego punktu, 0 <= i1 < #size().
		\param i2 Indeks pierwszego punktu, 0 <= i1 < #size().
		*/
		seconds_t get_interval_seconds(int i1, int i2) const;

		/** \brief Zwróć interwał czasowy w sekundach od pierwszego od i-tego punktu.
		\param i Indeks pierwszego punktu, 0 <= i < #size().
		*/
		seconds_t get_interval_from_start(const int i) const {
			assert(i < size());
			return intervals_from_start_[i];
		}

		/** \brief Zwróć wektor interwałów czasowych od początku harmonogramu do każdego punktu.
		*/
		std::vector<seconds_t> get_seconds() const {
			return intervals_from_start_;
		}

		/// Zwróć pierwszy punkt czasowy.
		DateTime get_start() const {
			return points_.front();
		}

		/// Zwróć ostatni punkt czasowy.
		DateTime get_end() const {
			return points_.back();
		}
	private:
		/// Punkty czasowe.
		std::vector<DateTime> points_;

		/// Interwały czasowe w sekundach od początku listy. Używane do przyspieszenia funkcji #get_interval_seconds.
		std::vector<seconds_t> intervals_from_start_;

		/// Liczba punktów.
		int size_;

		/// Wyczyść obiekt po przeniesieniu.
		void wipe();
	};	
}

namespace nlohmann {
	template <>
	struct adl_serializer<simbo::Schedule> {
		/// \brief Odczytaj obiekt Schedule z formatu JSON.
		/// Wymagane pola: start, period, nbr_points.
		static simbo::Schedule from_json(const json& j);
	};
}
