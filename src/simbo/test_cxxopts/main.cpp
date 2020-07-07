#include <cxxopts.hpp>

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options("test_cxxopts", "Test program for cxxopts");
		options.add_options()
			("r,reps", "Number of repetitions", cxxopts::value<int>())
			("n,name", "Name", cxxopts::value<std::string>())
			("h,help", "Help");
		options.parse_positional({ "reps" });

		const auto result = options.parse(argc, argv);

		if (result.count("help")) {
			std::cout << options.help({ "" }) << std::endl;
			return 0;
		}

		if (!result.count("name")) {
			throw std::runtime_error("No name provided");
		}
		const std::string name = result["name"].as<std::string>();
		const int reps = result["reps"].as<int>();
		if (reps < 0) {
			throw std::runtime_error("Negative repetition number");
		}
		for (int i = 0; i < reps; ++i) {
			std::cout << name << "\n";
		}

	} catch (std::exception& e) {
		std::cerr << e.what();
		return -1;
	}
}
