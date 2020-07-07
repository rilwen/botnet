#pragma once
#include "date_time.hpp"
#include "monotonic_piecewise_constant_interpolator1d.hpp"
#include "piecewise_constant_interpolator1d.hpp"
#include "json.hpp"

namespace simbo {
	template <template <class, class> class Interp, class RealX, class RealY> static void from_json_basic_implementation(const json& j, Interp<RealX, RealY>& interp, const char* class_name) {
		validate_keys(j, class_name, { "xs", "ys" }, {});
		try {
			std::vector<RealX> xs = j["xs"];
			std::vector<RealY> ys = j["ys"];
			interp = Interp<RealX, RealY>(std::move(xs), std::move(ys));
		} catch (std::exception& e) {
			throw DeserialisationError(class_name, j.dump(), e);
		}
	}

	template <class RealX, class RealY> void from_json(const json& j, PiecewiseConstantInterpolator1D<RealX, RealY>& interp) {
		from_json_basic_implementation(j, interp, "PiecewiseConstantInterpolator1D");
	}

	template <class RealX, class RealY> void from_json(const json& j, MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>& interp) {
		from_json_basic_implementation(j, interp, "MonotonicPiecewiseConstantInterpolator1D");
	}

	template <template <class, class> class Interp, class RealY> void from_json(const json& j, Interp<DateTime, RealY>& interp, const char* class_name) {
		if (j.is_object()) {
			from_json_basic_implementation(j, interp, class_name);
		} else if (j.is_primitive()) {
			std::vector<DateTime> xs({ min_datetime(), max_datetime() });
			std::vector<RealY> ys(1);
			ys[0] = j;
			interp = Interp<DateTime, RealY>(std::move(xs), std::move(ys));
		} else {
			throw DeserialisationError(class_name, j.dump(), boost::locale::translate("JSON data must be primitive or object"));
		}
	}

	template <class RealY> void from_json(const json& j, PiecewiseConstantInterpolator1D<DateTime, RealY>& interp) {
		from_json(j, interp, "PiecewiseConstantInterpolator1D");
	}

	template <class RealY> void from_json(const json& j, MonotonicPiecewiseConstantInterpolator1D<DateTime, RealY>& interp) {
		from_json(j, interp, "MonotonicPiecewiseConstantInterpolator1D");
	}

	template <class RealX, class RealY> void to_json_basic_implementation(json& j, const Interpolator1DImpl<RealX, RealY>& interp) {
		j = {
			{ "xs", interp.get_xs() },
			{ "ys", interp.get_ys() }
		};
	}

	template <class RealX, class RealY> void to_json(json& j, const PiecewiseConstantInterpolator1D<RealX, RealY>& interp) {
		to_json_basic_implementation(j, interp);
	}

	template <class RealX, class RealY> void to_json(json& j, const MonotonicPiecewiseConstantInterpolator1D<RealX, RealY>& interp) {
		to_json_basic_implementation(j, interp);
	}
}
