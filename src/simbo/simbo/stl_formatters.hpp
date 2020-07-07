#pragma once
#include <iosfwd>
#include <utility>
#include <vector>

namespace std {
	template <class V> std::ostream& operator<<(std::ostream& os, const std::vector<V>& v) {
		os << "[";
		if (!v.empty()) {
			auto it = v.begin();
			os << *it;
			++it;
			while (it != v.end()) {
				os << "," << *it;
				++it;
			}
		}
		os << "]";
		return os;
	}

	template <class A, class B> std::ostream& operator<<(std::ostream& os, const std::pair<A, B>& p) {
		os << "(" << p.first << "," << p.second << ")";
		return os;
	}
}
