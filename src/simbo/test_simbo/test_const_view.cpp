#include <vector>
#include <gtest/gtest.h>
#include "simbo/const_view.hpp"

using namespace simbo;

TEST(ConstView, vector) {
	std::vector<int*> original;
	int a = 0, b = 1, c = 2;
	original.push_back(&a);
	original.push_back(&b);
	original.push_back(&c);
	auto view = make_const_view(original);
	ASSERT_FALSE(view.empty());
	ASSERT_EQ(3, view.size());
	ASSERT_EQ(&a, view.front());
	ASSERT_EQ(&c, view.back());
	ASSERT_EQ(&b, view[1]);
	ASSERT_EQ(&b, view.at(1));
}
