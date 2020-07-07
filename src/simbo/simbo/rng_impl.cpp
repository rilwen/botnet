#include "rng_impl.hpp"

namespace simbo {
	RNGImpl::RNGImpl(int_type seed)
		: engine_(seed),
		uniform01_(0., 1.),
		normal01_(0., 1.) {

	}

	double RNGImpl::draw_uniform() {
		return uniform01_(engine_);
	}

	double RNGImpl::draw_gaussian() {
		return normal01_(engine_);
	}

	RNGImpl::int_type RNGImpl::draw_uniform_integer() {
		return engine_();
	}

	void RNGImpl::discard(unsigned long long z) {
		engine_.discard(z);
	}
}
