#pragma once
#include <tuple>
#include <utility>

namespace simbo {

	/// Złożenie dwóch haszów.
	inline size_t hash_combine(size_t lhs, size_t rhs) {
		lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
		return lhs;
	}

	/// Dostarcza funkcje haszujące dla różnych typów.
	struct Hasher {
	public:
		template <typename A, typename B>
		std::size_t operator()(const std::pair<A, B> &v) const
		{
			return hash_combine(operator()(v.first), operator()(v.second));
		}

		template <typename A, typename B, typename C>
		std::size_t operator()(const std::tuple<A, B, C> &v) const
		{
			return hash_combine(hash_combine(operator()(std::get<0>(v)), operator()(std::get<1>(v))), operator()(std::get<2>(v)));
		}

		template <typename X>
		std::size_t operator()(const X &v) const
		{
			return std::hash<X>()(v);
		}
	};
}
