#include "utils.hpp"

namespace simbo {
	std::string get_filename_extension(const std::string& filename) {
		const size_t dot_pos = filename.find_last_of('.');
		if (dot_pos != std::string::npos) {
			return filename.substr(dot_pos + 1);
		} else {
			return "";
		}
	}

	bool file_exists(const std::string& filename) {
		FILE* f;
		fopen_s(&f, filename.c_str(), "r");
		if (f) {
			fclose(f);
			return true;
		} else {
			return false;
		}
	}
}