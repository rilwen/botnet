#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <string>
#include <unordered_set>
#include "sinkhole_data_point.hpp"

namespace simbo {
	class Host;
	/** \brief Obiekt zbierający dane o botach które zostaną zaobserwowane przez obrońców Internetu.
	*/
	class Sinkhole {
	public:
		/** \brief Konstruktor.
		\param simulation_start Czas startu symulacji.
		\param save_full_data Czy zapisywać pełne dane.
		*/
		Sinkhole(DateTime simulation_start, bool save_full_data);

		Sinkhole(Sinkhole&&) = default;		

		/// Zrzuć pełne dane do strumienia wyjściowego, w formacie CSV.
		/// \param out Strumień wyjściowy.
		void dump_full_csv(std::ostream& out) const;		

		/// Zrzuć pełne dane do strumienia wyjściowego w binarnym formacie Simbo.
		/// Funkcja służy do zapisania wszystkich danych po zakończeniu symulacji. Dane nie są usuwane z obiektu Sinkhole.
		/// \param out Strumień wyjściowy.
		void dump_full_bin(std::ostream& out) const;

		/// Zrzuć pełne dane do strumienia wyjściowego w binarnym formacie Simbo.
		/// Funkcja służy do zapisywania wszystkich danych co pewien czas w trakcie albo po symulacji. Zapisane dane są usuwane z pamięci.
		/// \param out Strumień wyjściowy. Funkcja zakłada, że w każdym wywołaniu dostanie ten sam strumień.
		void dump_full_bin_incremental(std::ostream& out);

		/// Zrzuć zredukowane dane do strumienia wyjściowego, w formacie CSV.
		/// \param out Strumień wyjściowy.
		void dump_reduced_csv(std::ostream& out) const;

		/// Numer wersji formatu binarnego.
		static const int binary_format_version = 3;

		typedef SinkholeDataPoint<binary_format_version> data_point_t;

		/** \brief Pobierz punkt danych z bota.
		\param host Wskaźnik do bota.
		\param time Czas złapania.
		\return Dane.
		\throw std::invalid_argument Jeżeli host jest null, nie ma ustalonego adresu IP albo nie ma przydzielonej roli w botnecie.
		*/
		data_point_t get_data_from_bot(const Host* host, seconds_t time) const;

		/// Dodaj nowy punkt danych.
		void add_data_point(data_point_t&& data_point);

		/// Zwróć liczbę zebranych punktów danych.
		int get_number_data_points() const {
			return static_cast<int>(data_.size());
		}

		/// Zwróć liczbę zebranych punktów danych które zostały zapisane poprzez wywołania funkcji #dump_full_bin_incremental().
		int get_number_dumped_data_points() const {
			return static_cast<int>(nbr_data_points_dumped_);
		}

		/// Data od której liczone są sekundy zapisywane w sinkhole.
		static const DateTime EPOCH;

		/// Godzina jaką oznaczamy zredukowane dane dobowe w pliku CSV pisanym przez funkcję #dump_reduced_csv().
		static constexpr int HOUR_FOR_DAILY_DATA = -1;

		/// Zapisz nagłówek do pliku binarnego.
		/// \param number_rows Liczba wierszy danych.
		/// \param out Strumień wyjścia do pliku binarnego.
		static void write_header(size_t number_rows, std::ostream& out);

		/// Akumulator zredukowanych danych z obecnego okresu (godzina, dzień, tydzień, itd.) i kraju.
		class ReducedDataAccumulator {
		public:
			/// Domyślny konstruktor.
			ReducedDataAccumulator();

			void add_data_point(data_point_t::ip_t ip_address, data_point_t::host_id_t host_id);

			const std::unordered_set<data_point_t::ip_t>& get_ip_addresses() const {
				return ip_addresses_;
			}

			const std::unordered_set<data_point_t::host_id_t>& get_host_ids() const {
				return host_ids_;
			}

			int64_t get_message_count() const {
				return message_count_;
			}

			void reset();

		private:
			/// Adresy IP.
			std::unordered_set<data_point_t::ip_t> ip_addresses_;
			/// ID hostów.
			std::unordered_set<data_point_t::host_id_t> host_ids_;
			/// Liczba wiadomości.
			int64_t message_count_;
		};

		/// Zredukowane dane dla pojedynczego kraju z pewnego okresu: liczba adresów IP, hostów i wiadomości.
		struct ReducedData {
			/// Konstruktor domyślny.
			ReducedData();

			/// Skopiuj dane z akumulatora.
			ReducedData& operator=(const ReducedDataAccumulator& accumulator);

			/// Liczba adresów IP.
			int64_t ip_address_count;

			/// Liczba hostów.
			int64_t host_count;

			/// Liczba wiadomości.
			int64_t message_count;
		};

	private:		

		/// Akumulatory na zredukowane dane dla wszystkich krajów świata.
		typedef std::array<ReducedDataAccumulator, std::numeric_limits<data_point_t::country_code_t>::max() + 1> global_reduced_data_accumulator_t;

		/// Zresetuj wszystkie akumulatorki.
		static void reset_global_reduced_data_accumulator(global_reduced_data_accumulator_t& global_reduced_data_accumulator);

		/// Zredukowane dane dla wszystkich krajów świata dla danego dnia i godziny, albo dla całego dnia.
		struct GlobalReducedDataWithTime {
			/// Skopiuj dane z akumulatora.
			/// \param hour Godzina akumulacji. Jeżeli hour == HOUR_FOR_DAILY_DATA, to akumulacja obejmowała cały dzień date.
			GlobalReducedDataWithTime(const global_reduced_data_accumulator_t& accumulator, const Date& date, int hour);

			/// Dane.
			std::array<ReducedData, std::numeric_limits<data_point_t::country_code_t>::max() + 1> data;

			/// Data.
			Date date;

			/// Godzina (HOUR_FOR_DAILY_DATA dla danych dobowych).
			int hour;
		};

		/// Nagłówek formatu binarnego.
		struct Header {
			Header(size_t number_rows);
			static const int reserved_size = 96;
			int8_t marker[8] = { 'S', 'I', 'M', 'B', 'O', 'B', 'I', 'N' };
			uint64_t version = binary_format_version;
			uint64_t row_size = sizeof(data_point_t);
			uint64_t number_rows;
			int8_t reserved[reserved_size];
		};

		void write_global_reduced_rows_csv(std::ostream& out, const GlobalReducedDataWithTime& reduced_data_row) const;

		/// Usuń zbuforowane pełne dane.
		void wipe_full_data();

		/** Zmienne. **/

		/// Liczba zbuforowanych danych zapisanych już do pliku. Używana wyłącznie przez #dump_full_bin_incremental().
		size_t nbr_data_points_dumped_;

		/// Zbuforowane dane.
		std::vector<data_point_t> data_;

		/// Czas startu symulacji.
		DateTime simulation_start_;

		/// Czy zapisywać pełne dane.
		bool save_full_data_;

		/// Data dla której akumulujemy dane.
		Date accumulation_date_;

		/// Akumulatory na dane z obecnego dnia.
		global_reduced_data_accumulator_t global_reduced_daily_data_accumulator_;

		/// Godzina accumulation_date_ dla której akumulujemy dane.
		int accumulation_hour_;

		/// Akumulatory na dane z obecnej godziny.
		global_reduced_data_accumulator_t global_reduced_hourly_data_accumulator_;

		/// Wektor zredukowanych danych podsumowujących kolejne dni i godziny symulacji dla wszystkich krajów.
		std::vector<GlobalReducedDataWithTime> global_reduced_data_;
	};
}
