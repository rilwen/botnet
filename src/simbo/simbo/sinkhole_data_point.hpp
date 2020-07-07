#pragma once
#include "bot_state.hpp"
#include "date_time.hpp"
#include "id.hpp"
#include "ip_address.hpp"
#include "local_network_type.hpp"

namespace simbo {
	/// Punkt danych w formacie binarnym dla Sinkhole.
	template <int Version> struct SinkholeDataPoint {};

	/// Punkt danych (wersja 1).
	template <> struct SinkholeDataPoint<1> {
		/// Typ adresu IP.
		typedef int64_t ip_t;

		/// Typ ID hosta.
		typedef int32_t host_id_t;

		/** Konstruktor.
		\param time Czas obserwacji w sekundach od Sinkhole::EPOCH.
		\param ip Adres IP bota.
		\param host_id ID bota.
		\param bot_state Stan bota który wysłał wiadomość, w postaci liczbowej.
		\param local_network_type Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		\param is_fixed Czy bot jest desktopem (true), czy laptopem (false).
		*/
		SinkholeDataPoint(seconds_t time, IpAddress ip, Id::id_t host_id, BotState bot_state, LocalNetworkType local_network_type, bool is_fixed);

		SinkholeDataPoint(SinkholeDataPoint&&) = default;

		/// Czas obserwacji w sekundach od Sinkhole::EPOCH.
		double time;

		/// Adres IP bota w postaci liczbowej.
		ip_t ip;

		/// ID bota.
		host_id_t host_id;

		/// Stan bota który wysłał wiadomość, w postaci liczbowej.
		int8_t bot_state;

		/// Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		int8_t local_network_type;

		/// Czy bot jest desktopem (1), czy laptopem (0).
		int8_t is_fixed;

	private:
		/// Wypełnienie (zawsze zero)
		int8_t zero_pad_;
	};
	static_assert(sizeof(SinkholeDataPoint<1>) == 24, "Size of the SinkholeDataPoint<1> struct is expected to be 24 bytes");


	/// Punkt danych (wersja 2).
	template <> struct SinkholeDataPoint<2> {
		/// Typ adresu IP.
		typedef int64_t ip_t;

		/// Typ ID hosta.
		typedef int32_t host_id_t;

		/// Typ kodu kraju.
		typedef uint8_t country_code_t;

		/** Konstruktor.
		\param time Czas obserwacji w sekundach od Sinkhole::EPOCH.
		\param ip Adres IP bota.
		\param host_id ID bota.
		\param bot_state Stan bota który wysłał wiadomość, w postaci liczbowej.
		\param local_network_type Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		\param is_fixed Czy bot jest desktopem (true), czy laptopem (false).
		\param country Kod kraju (w zakresie 0 do 255).
		\throw std::domain_error Jeżeli country nie mieści się w zakresie [0, 255].
		*/
		SinkholeDataPoint(seconds_t time, IpAddress ip, Id::id_t host_id, BotState bot_state, LocalNetworkType local_network_type, bool is_fixed, int country);

		SinkholeDataPoint(SinkholeDataPoint&&) = default;

		/// Czas obserwacji w sekundach od Sinkhole::EPOCH.
		double time;

		/// Adres IP bota w postaci liczbowej.
		ip_t ip;

		/// ID bota.
		host_id_t host_id;

		/// Stan bota który wysłał wiadomość, w postaci liczbowej.
		int8_t bot_state;

		/// Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		int8_t local_network_type;

		/// Czy bot jest desktopem (1), czy laptopem (0).
		int8_t is_fixed;

		/// Kod kraju.
		country_code_t country;
	};
	static_assert(sizeof(SinkholeDataPoint<2>) == 24, "Size of the SinkholeDataPoint<2> struct is expected to be 24 bytes");

	/// Punkt danych (wersja 3).
	template <> struct SinkholeDataPoint<3> {
		/// Typ adresu IP.
		typedef int64_t ip_t;

		/// Typ ID hosta.
		typedef int32_t host_id_t;

		/// Typ kodu kraju.
		typedef uint8_t country_code_t;

		/// Typ numeru edycji payloadu.
		typedef uint8_t payload_release_number_t;

		/** Konstruktor.
		\param time Czas obserwacji w sekundach od Sinkhole::EPOCH.
		\param ip Adres IP bota.
		\param host_id ID bota.
		\param bot_state Stan bota który wysłał wiadomość, w postaci liczbowej.
		\param local_network_type Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		\param is_fixed Czy bot jest desktopem (true), czy laptopem (false).
		\param country Kod kraju (w zakresie 0 do 255).
		\param payload_release_number Numer edycji payloadu (w zakresie 0 do 255).
		\throw std::domain_error Jeżeli country albo payload_release_number nie mieści się w zakresie [0, 255].
		*/
		SinkholeDataPoint(seconds_t time, IpAddress ip, Id::id_t host_id, BotState bot_state, LocalNetworkType local_network_type, bool is_fixed, int country, int payload_release_number);

		SinkholeDataPoint(SinkholeDataPoint&&) = default;

		/// Czas obserwacji w sekundach od Sinkhole::EPOCH.
		double time;

		/// Adres IP bota w postaci liczbowej.
		ip_t ip;

		/// ID bota.
		host_id_t host_id;

		/// Stan bota który wysłał wiadomość, w postaci liczbowej.
		int8_t bot_state;

		/// Typ sieci z której została wysłana wiadomość, w postaci liczbowej.
		int8_t local_network_type;

		/// Czy bot jest desktopem (1), czy laptopem (0).
		int8_t is_fixed;

		/// Kod kraju.
		country_code_t country;

		/// Numer edycji payloadu.
		payload_release_number_t payload_release;
	private:
		/// Wypełnienie (zawsze zero)
		int8_t zero_pad_[7];
	};
	static_assert(sizeof(SinkholeDataPoint<3>) == 32, "Size of the SinkholeDataPoint<3> struct is expected to be 32 bytes");
}
