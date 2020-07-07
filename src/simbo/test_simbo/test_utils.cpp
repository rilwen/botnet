#include <gtest/gtest.h>
#include "simbo/utils.hpp"

using namespace simbo;

TEST(Utils, make_copy) {
	const std::string str1("ala ma kota");
	const auto str2 = make_copy(str1);
	ASSERT_EQ(str1, str2);
	ASSERT_NE(&str1, &str2);
}

TEST(Utils, find_left_node) {
	const std::vector<double> v({ 0, 1, 2 });
	ASSERT_EQ(v.begin(), find_left_node(v.begin(), v.end(), 0));
	ASSERT_EQ(v.begin(), find_left_node(v.begin(), v.end(), 0.5));
	ASSERT_EQ(v.begin() + 1, find_left_node(v.begin(), v.end(), 1));
	ASSERT_EQ(v.begin() + 1, find_left_node(v.begin(), v.end(), 1.5));
	ASSERT_EQ(v.begin() + 2, find_left_node(v.begin(), v.end(), 2));
	ASSERT_EQ(v.begin() + 2, find_left_node(v.begin(), v.end(), 2.1));
	ASSERT_THROW(find_left_node(v.begin(), v.begin(), 0), std::invalid_argument);
	ASSERT_THROW(find_left_node(v.begin(), v.end(), -0.1), std::domain_error);
}

TEST(Utils, find_right_node) {
	const std::vector<double> v({ 0, 1, 2 });
	ASSERT_EQ(v.begin(), find_right_node(v.begin(), v.end(), -0.1));
	ASSERT_EQ(v.begin(), find_right_node(v.begin(), v.end(), 0));
	ASSERT_EQ(v.begin() + 1, find_right_node(v.begin(), v.end(), 0.5));
	ASSERT_EQ(v.begin() + 1, find_right_node(v.begin(), v.end(), 1));
	ASSERT_EQ(v.begin() + 2, find_right_node(v.begin(), v.end(), 1.5));
	ASSERT_EQ(v.begin() + 2, find_right_node(v.begin(), v.end(), 2));
	ASSERT_THROW(find_right_node(v.begin(), v.begin(), 0), std::invalid_argument);
	ASSERT_THROW(find_right_node(v.begin(), v.end(), 2.1), std::domain_error);
}

TEST(Utils, repeat_elements) {
	ASSERT_EQ(std::vector<int>({ 0, 1, 2, 0, 1 }), repeat_elements(std::vector<int>({ 0, 1, 2 }), 5));
}

TEST(Utils, get_filename_extension) {
	ASSERT_EQ("h5", get_filename_extension("data.h5"));
	ASSERT_EQ("", get_filename_extension("README"));
	ASSERT_EQ("", get_filename_extension("README."));
}
