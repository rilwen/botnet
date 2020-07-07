#pragma once
#include <memory>
#include <boost/locale.hpp>
#include "interpolator1d.hpp"

namespace simbo {
	/**
	\brief Interpolator1D który wybiera pomiędzy obiektami z tablicy i zwraca referencję do wybranego.

	Metoda interpolacji: wartość(x) = wektor_wartości[interpolator_indeksów(x)]. Jeżeli index := interpolator_indeksów(x) przekracza
	zakres indeksów wektora wartości, zwróć pierwszą (dla index < 0) albo ostatnią (dla index >= wektor_wartości.size()) wartość z wektor_wartości.

	\tparam RealX Argument interpolatora.
	\tparam ValueY Typ obiektów do których zwracamy referencje.	
	*/
	template <class RealX, class ValueY> class IndexedInterpolator1D: public Interpolator1D<RealX, const ValueY&> {
	public:
		/// Skrócone nazwy typów.
		typedef int index_t;
		typedef Interpolator1D<RealX, index_t> IndexInterpolator;
		typedef std::unique_ptr<const IndexInterpolator> index_interpolator_ptr;

		/**
		\brief Konstruktor.

		\param values Wektor interpolowanych wartości.
		\param index_interpolator Interpolator indeksów.

		\throws std::invalid_argument Jeżeli index_interpolator == nullptr albo values.empty().
		*/
		IndexedInterpolator1D(std::vector<ValueY>&& values, index_interpolator_ptr&& index_interpolator)
			: max_index_(static_cast<int>(values.size()) - 1) {
			if (!index_interpolator) {
				throw std::invalid_argument(boost::locale::translate("Null index interpolator pointer"));
			}
			if (values.empty()) {
				throw std::invalid_argument(boost::locale::translate("Empty values vector"));
			}
			values_ = std::move(values);
			index_interpolator_ = std::move(index_interpolator);
		}

		RealX get_lower_bound() const override {
			return index_interpolator_->get_lower_bound();
		}

		RealX get_upper_bound() const override {
			return index_interpolator_->get_upper_bound();
		}

		int size() const override {
			return index_interpolator_->size();
		}

		int find_left_node_index(RealX x) const override {
			return index_interpolator_->find_left_node_index(x);
		}

		const ValueY& evaluate(RealX x, int left_node_index) const override {
			assert(left_node_index >= 0);
			assert(left_node_index < size());
			const auto index = std::max(0, std::min(max_index_, index_interpolator_->evaluate(x, left_node_index)));
			return values_[index];
		}
	private:
		std::vector<ValueY> values_;
		index_interpolator_ptr index_interpolator_;
		int max_index_;
	};
}
