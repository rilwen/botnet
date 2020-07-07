#include <boost/math/special_functions.hpp>
#include "mock_rng.hpp"

namespace simbo {
	MockRNG::MockRNG()
		: number_draws_(0) {

	}

	double MockRNG::draw_uniform() {
		++number_draws_;
		return values_.next();
	}

	double MockRNG::draw_gaussian() {
		return -1.4142135623730950488 * boost::math::erfc_inv(2 * draw_uniform());
	}

	void MockRNG::add_uniform(double u) {
		// Nie sprawdzamy zakresu, żeby móc oszukiwać w testach.
		values_.add(u);
	}

	void MockRNG::rewind() {
		values_.reset();
	}

	void MockRNG::reset() {
		rewind();
		number_draws_ = 0;
	}

	void MockRNG::clear() {
		values_.clear();
		number_draws_ = 0;
	}

	void MockRNG::add_from(RNG& other, int n) {
		assert(n >= 0);
		for (int i = n - 1; i >= 0; --i) {
			values_.add(other.draw_uniform());
		}
	}

	void MockRNG::discard(const unsigned long long z) {
		for (unsigned long long i = 0; i < z; ++i) {
			values_.next();
		}
	}

	MockRNG::int_type MockRNG::draw_uniform_integer() {
		return static_cast<MockRNG::int_type>(std::round(static_cast<double>(get_int_max()) * draw_uniform()));
	}
}
