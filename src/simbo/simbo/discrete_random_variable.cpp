#include <numeric>
#include <boost/locale.hpp>
#include "discrete_random_variable.hpp"
#include "json.hpp"
#include "rng.hpp"

namespace simbo {
	DiscreteRandomVariable::DiscreteRandomVariable()
		: cdf_({ 1. }), min_(0) {
	}

	DiscreteRandomVariable::DiscreteRandomVariable(const std::vector<double>& weights, int min)
		: cdf_(weights), min_(min), max_(min + static_cast<int>(weights.size()) - 1) {
		if (weights.empty()) {
			throw std::invalid_argument(boost::locale::translate("Weights vector is empty"));
		}
		if (std::any_of(weights.begin(), weights.end(), [](double w) { return !(w >= 0); })) {
			throw std::invalid_argument(boost::locale::translate("Weights must be non-negative"));
		}
		const double sum_weights = std::accumulate(cdf_.begin(), cdf_.end(), 0.);
		if (!sum_weights) {
			throw std::invalid_argument(boost::locale::translate("Weights have zero sum"));
		}
		std::for_each(cdf_.begin(), cdf_.end(), [sum_weights](double& weight) { weight /= sum_weights; });
		std::partial_sum(cdf_.begin(), cdf_.end(), cdf_.begin());
		std::for_each(cdf_.begin(), cdf_.end(), [](double& p) { p = std::min(p, 1.); });
		cdf_.back() = 1.;
	}

	DiscreteRandomVariable::DiscreteRandomVariable(const DiscreteRandomVariable& other)
		: cdf_(other.cdf_), min_(other.min_), max_(other.max_) {

	}

	DiscreteRandomVariable::DiscreteRandomVariable(DiscreteRandomVariable&& other)
		: cdf_(std::move(other.cdf_)), min_(other.min_), max_(other.max_) {
	}

	DiscreteRandomVariable& DiscreteRandomVariable::operator=(const DiscreteRandomVariable& other) {
		if (this != &other) {
			cdf_ = other.cdf_;
			min_ = other.min_;
			max_ = other.max_;
		}
		return *this;
	}

	int DiscreteRandomVariable::operator()(RNG& rng) const {
		const double u = rng.draw_uniform();
		auto it = std::lower_bound(cdf_.begin(), cdf_.end(), u);
		assert(it != cdf_.end());
		if (u < 1) {
			if ((it + 1) != cdf_.end() && *it == u) {
				++it;
			}
		}
		return min_ + static_cast<int>(std::distance(cdf_.begin(), it));
	}

	double DiscreteRandomVariable::get_probability(const int i) const {
		if (i < min() || i > max()) {
			throw std::domain_error((boost::locale::format("Value {1} outside the range [{2}, {3}] of this random variable") % i % min() % max()).str());
		}
		const auto j = i - min();
		if (j) {
			return cdf_[j] - cdf_[j - 1];
		} else {
			return cdf_.front();
		}
	}

	void from_json(const json& j, std::unique_ptr<DiscreteRandomVariable>& rv) {
		validate_keys(j, "DiscreteRandomVariable", { "weights", "min" }, {});
		try {
			const std::vector<double> weights = j["weights"];
			const int min = j["min"];
			rv = std::make_unique<DiscreteRandomVariable>(weights, min);
		} catch (std::exception& e) {
			throw DeserialisationError("DiscreteRandomVariable", j.dump(), e);
		}
	}
}
