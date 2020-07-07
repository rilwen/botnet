#include <gtest/gtest.h>
#include "simbo/data_storage_format.hpp"

using namespace simbo;

TEST(DataStorageFormat, data_storage_format_from_extension) {
	ASSERT_EQ(DataStorageFormat::CSV, data_storage_format_from_extension("csv"));
	ASSERT_EQ(DataStorageFormat::BIN, data_storage_format_from_extension("bin"));
	ASSERT_THROW(data_storage_format_from_extension("xls"), std::invalid_argument);
}