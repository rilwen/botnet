#include <gtest/gtest.h>
#include "simbo/store.hpp"

using namespace simbo;

TEST(Store, test) {
	Store<int> store;
	ASSERT_TRUE(store.empty());
	ASSERT_EQ(0, store.size());
	ASSERT_FALSE(store.has("foo"));
	ASSERT_THROW(store.has(""), std::invalid_argument);
	ASSERT_THROW(store.get("foo"), std::invalid_argument);
	ASSERT_THROW(store.get(""), std::invalid_argument);
	ASSERT_THROW(store.get("", 0), std::invalid_argument);
	ASSERT_EQ(-1, store.get("foo", -1));
	ASSERT_THROW(store.add("", 2), std::invalid_argument);
	store.add("one", 1);
	ASSERT_FALSE(store.empty());
	ASSERT_EQ(1, store.size());
	ASSERT_THROW(store.add("one", 2), std::invalid_argument);
	ASSERT_TRUE(store.has("one"));
	ASSERT_EQ(1, store.get("one"));
	ASSERT_EQ(1, store.get("one", 0));
	store.add("another_one", 1);
	ASSERT_EQ(2, store.size());
}
