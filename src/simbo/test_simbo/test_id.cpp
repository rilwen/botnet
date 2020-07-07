#include <random>
#include <gtest/gtest.h>
#include "simbo/id.hpp"

using namespace simbo;

TEST(Id, test) {
	ASSERT_LT(Id::min_good_id(), Id::max_good_id());
	std::mt19937 rng(42);
	const std::uniform_int_distribution<Id::id_t> distr(Id::min_good_id(), Id::max_good_id());
	for (int i = 0; i < 1000; ++i) {
		const Id::id_t id = distr(rng);
		ASSERT_TRUE(Id::is_good(id)) << id;
		ASSERT_FALSE(Id::is_special(id)) << id;
		ASSERT_NE(Id::undefined_id(), id) << id;
	}
}

TEST(Id, get_next_good_id) {
	Id::id_t next_id = 10;
	ASSERT_EQ(10, Id::get_next_good_id(next_id));
	ASSERT_EQ(11, next_id);
}

TEST(Id, get_next_good_id_undefined) {
	Id::id_t next_id = Id::undefined_id();
	ASSERT_THROW(Id::get_next_good_id(next_id), std::domain_error);
}

TEST(Id, get_next_good_id_too_low) {
	Id::id_t next_id = -100;
	ASSERT_THROW(Id::get_next_good_id(next_id), std::domain_error);
}

TEST(Id, get_next_good_id_ran_out) {
	Id::id_t next_id = Id::max_good_id();
	Id::get_next_good_id(next_id);
	ASSERT_EQ(Id::undefined_id(), next_id);
}
