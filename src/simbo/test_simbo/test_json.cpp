#include <gtest/gtest.h>
#include "simbo/json.hpp"
#include "simbo/store.hpp"

using namespace simbo;

namespace simbo {
	struct SomeClassOrOther {
		SomeClassOrOther(int nx) {
			x = nx;
		}

		int x;
	};

	void from_json(const json& j, std::unique_ptr<SomeClassOrOther>& ptr) {
		int x = j.at("x");
		ptr = std::make_unique<SomeClassOrOther>(x);
	}
}

TEST(Json, from_json_const_unique_ptr) {
	std::unique_ptr<const SomeClassOrOther> ptr;
	json j = "{\"x\": 3}"_json;
	from_json(j, ptr);
	ASSERT_NE(nullptr, ptr);
	ASSERT_EQ(3, ptr->x);
}

TEST(Json, from_json_shared_ptr) {
	std::shared_ptr<SomeClassOrOther> ptr;
	json j = "{\"x\": 3}"_json;
	from_json(j, ptr);
	ASSERT_NE(nullptr, ptr);
	ASSERT_EQ(3, ptr->x);
}

TEST(Json, from_json_shared_ptr_const) {
	std::shared_ptr<const SomeClassOrOther> ptr;
	json j = "{\"x\": 3}"_json;
	from_json(j, ptr);
	ASSERT_NE(nullptr, ptr);
	ASSERT_EQ(3, ptr->x);
}

TEST(Json, from_json_with_store) {
	Store<std::shared_ptr<SomeClassOrOther>> store;
	const auto two = std::make_shared<SomeClassOrOther>(2);
	store.add("two", two);
	json j = "{\"x\": 1}"_json;
	std::shared_ptr<SomeClassOrOther> ptr;
	from_json_or_store(j, store, ptr);
	ASSERT_NE(nullptr, ptr);
	ASSERT_EQ(1, ptr->x);
	j = "\"two\""_json;
	from_json_or_store(j, store, ptr);
	ASSERT_EQ(two.get(), ptr.get());
}

TEST(Json, expect_key) {
	json j = "{\"x\": 1}"_json;
	ASSERT_NO_THROW(expect_key(j, "", "x"));
	ASSERT_THROW(expect_key(j, "", "y"), DeserialisationError);
}

TEST(Json, validate_keys) {
	json j = "{\"x\": 1, \"y\": \"a\"}"_json;
	ASSERT_NO_THROW(validate_keys(j, "", { "x" }, { "y" }));
	ASSERT_NO_THROW(validate_keys(j, "", { "y" }, { "x", "z" }));
	ASSERT_NO_THROW(validate_keys(j, "", { "x", "y" }, { }));
	ASSERT_THROW(validate_keys(j, "", { "x" }, {}), DeserialisationError);
	ASSERT_THROW(validate_keys(j, "", { "x" }, {"z"}), DeserialisationError);
	ASSERT_THROW(validate_keys(j, "", { "x", "y", "z" }, {}), DeserialisationError);
	ASSERT_THROW(validate_keys(j, "", { "x", "z" }, {"y"}), DeserialisationError);
	j = "\"ala\"";
	ASSERT_THROW(validate_keys(j, "", { "x" }, {}), DeserialisationError);
}
