#include <iostream>
#include <boost/stacktrace.hpp>
#include <boost/exception/all.hpp>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

typedef boost::error_info<struct tag_stacktrace, boost::stacktrace::stacktrace> traced;

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	spdlog::set_level(spdlog::level::debug);
	try {
		return RUN_ALL_TESTS();
	} catch (const std::exception& e) {
		// Uruchom testy z opcją --gtest_catch_exceptions=0 żeby zobaczyć stacktrace.
		std::cerr << e.what() << '\n';
		const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
		if (st) {
			std::cerr << *st << '\n';
		} else {
			std::cerr << "No stack trace..." << std::endl;
		}
		throw;
	}
}
