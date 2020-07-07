#pragma once
#include <cassert>

namespace simbo {
	/// Widok na kontener / iterator C++ zawierający / wskazujący na wskaźniki albo referencje non-const, konwertujący je do wersji const.
	template <class C> class ConstView {
	};

	/// Funkcja pomocnicza tworząca ConstView.
	template <class C> ConstView<C> make_const_view(const C& v) {
		return ConstView<C>(v);
	}

	/// Widok na kontener C++ (np. std::vector) zawierający wskaźniki albo referencje non-const konwertujący je na ich wersje const.
	template <class C> class BaseContainerConstView {
	public:
		BaseContainerConstView(const C& container)
			: container_(container) {

		}

		size_t size() const {
			return container_.size();
		}

		bool empty() const {
			return container_.empty();
		}
	protected:
		const C& container_;
	};

	
	/// Specjalizacja ConstView dla wektora wskaźników.
	template <class T> class ConstView<std::vector<T*>>: public BaseContainerConstView<std::vector<T*>> {
	public:
		using BaseContainerConstView<std::vector<T*>>::BaseContainerConstView;

		const T* front() const {
			return container_.front();
		}

		const T* back() const {
			return container_.back();
		}

		const T* operator[](size_t i) const {
			assert(i < size());
			return container_[i];
		}

		const T* at(size_t i) const {
			return container_.at(i);
		}
	};

	
}
