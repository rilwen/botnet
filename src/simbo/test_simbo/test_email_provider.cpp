#include <cmath>
#include <gtest/gtest.h>
#include "simbo/email_provider.hpp"

using namespace simbo;

TEST(EmailProvider, constructor) {
	EmailProvider ep("poczta", 0.95);
	ASSERT_EQ(0.95, ep.get_email_filtering_efficiency());
	ASSERT_EQ("poczta", ep.get_name());
}

TEST(EmailProvider, constructor_throws) {
	const std::string name("poczta");
	ASSERT_THROW(EmailProvider(name, -0.1), std::domain_error);
	ASSERT_THROW(EmailProvider(name, 1.1), std::domain_error);
	ASSERT_THROW(EmailProvider(name, nan("")), std::domain_error);
}
