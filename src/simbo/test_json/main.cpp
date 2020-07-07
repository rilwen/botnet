#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class Species {
	DOG,
	OCTOPUS
};

void from_json(const json& j, Species& sp) {
	std::string s = j;
	if (s == "DOG") {
		sp = Species::DOG;
	} else if (s == "OCTOPUS") {
		sp = Species::OCTOPUS;
	} else {
		throw std::invalid_argument("Unknown species");
	}
}

void to_json(json& j, const Species sp) {
	if (sp == Species::DOG) {
		j = "DOG";
	} else {
		j = "OCTOPUS";
	}
}

int main() {
	json j;
	std::unordered_map<Species, bool> is_aquatic;
	is_aquatic[Species::DOG] = false;
	is_aquatic[Species::OCTOPUS] = true;
	j = is_aquatic;
	std::cout << j << std::endl;
	j = json();
	j["size"] = 1;
	j["species"] = Species::OCTOPUS;
	j["colour"] = "red";
	j["is_aquatic"] = is_aquatic;
	json details;
	details["alive"] = true;
	details["genes"] = { 'G', 'A', 'T', 'T', 'A', 'C', 'A' };
	details["traits"] = json::array();
	details["traits"].push_back(1);
	details["traits"].push_back("foo");
	j["details"] = details;
	std::ofstream outf("test.json");
	outf << std::setw(4) << j << std::endl;
	outf.close();

	std::ifstream inf("test.json");
	json j2;
	inf >> j2;
	if (j == j2) {
		std::cout << "Equal data\n";
	} else {
		std::cout << "Not equal data\n";
	}
	
}
