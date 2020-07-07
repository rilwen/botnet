#pragma once
#include <unordered_map>
#include "date_time.hpp"
#include "hasher.hpp"
#include "monotonic_piecewise_constant_interpolator1d.hpp"

namespace simbo {
	/// Konfiguracja dostawcy oprogramowania.
	/// \tparam Spec \see SoftwareVendor.
	template <class Spec> struct SoftwareVendorConfig {
		/// Typ harmonogramu wypuszczania nowych edycji konkretnej wersji oprogramowania.
		typedef MonotonicPiecewiseConstantInterpolator1D<DateTime, int> release_schedule_t;

		/// Typ mapy harmonogramów edycji dla wszystkich wersji oprogramowania.
		typedef std::unordered_map<Spec, release_schedule_t, Hasher> release_map_t;

		/// Wirtualny destruktor.
		virtual ~SoftwareVendorConfig() {}

		/// Harmonogram uaktualniania edycji systemów dostarczanych przez dostawcę.
		/// Interpolatory muszą być ściśle rosnące.
		release_map_t release_schedules;

		/** \brief Sprawdź poprawność konfiguracji.
		\throws std::domain_error Jeżeli jakiś parameter jest poza swoją dziedziną matematyczną.
		\throws std::invalid_argument Jeżeli któryś parameter nie spełnia innego warunku.
		*/
		virtual void validate() const;
	};

	template <class Spec> void SoftwareVendorConfig<Spec>::validate() const {
		for (const auto& kv : release_schedules) {
			const auto& spec = kv.first;
			const auto& release_interpolator = kv.second;
			const Monotonicity monotonicity = release_interpolator.get_monotonicity();
			if (release_interpolator.size() > 1 && monotonicity != Monotonicity::STRICTLY_INCREASING) {
				throw std::invalid_argument((
					boost::locale::format("Release numbers for software {1} are not strictly increasing: {2}")
					% spec % monotonicity).str());
			}
		}
	}	
}