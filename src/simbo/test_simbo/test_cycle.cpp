#include <gtest/gtest.h>
#include "simbo/cycle.hpp"

using namespace simbo;

TEST(Cycle, constructor) {
	Cycle<int> cycle;
	ASSERT_TRUE(cycle.empty());
}

TEST(Cycle, throw_if_empty) {
	Cycle<int> cycle;
	ASSERT_THROW(cycle.next(), std::logic_error);
}

TEST(Cycle, add) {
	Cycle<int> cycle;
	cycle.add(1);
	ASSERT_FALSE(cycle.empty());
}

TEST(Cycle, add_and_next) {
	Cycle<int> cycle;
	cycle.add(1);
	ASSERT_EQ(1, cycle.next());
	ASSERT_EQ(1, cycle.next());
	cycle.add(-1);
	ASSERT_EQ(1, cycle.next());
	ASSERT_EQ(-1, cycle.next());
	ASSERT_EQ(1, cycle.next());
	cycle.add(4);
	ASSERT_EQ(-1, cycle.next());
	ASSERT_EQ(4, cycle.next());
	ASSERT_EQ(1, cycle.next());
}

TEST(Cycle, reset) {
	Cycle<int> cycle;
	cycle.add(1);
	cycle.add(-1);
	cycle.add(4);
	cycle.add(40);
	ASSERT_EQ(1, cycle.next());
	ASSERT_EQ(-1, cycle.next());
	cycle.reset();
	ASSERT_EQ(1, cycle.next());
	ASSERT_EQ(-1, cycle.next());
	ASSERT_EQ(4, cycle.next());
	ASSERT_EQ(40, cycle.next());
}

TEST(Cycle, clear) {
	Cycle<int> cycle;
	cycle.add(1);
	cycle.add(-1);
	cycle.clear();
	ASSERT_TRUE(cycle.empty());
	ASSERT_THROW(cycle.next(), std::logic_error);
	cycle.add(1);
	cycle.add(-1);
	ASSERT_EQ(1, cycle.next());
}