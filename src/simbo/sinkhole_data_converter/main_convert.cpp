#include <cstdio>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <cxxopts.hpp>
#include <boost/system/config.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "simbo/ip_address.hpp"
#include "simbo/json.hpp"
#include "simbo/log.hpp"
#include "simbo/n6.hpp"
#include "simbo/utils.hpp"

/*
Program konwertujący dane zebrane przez sinkhole z formatu JSON do binarnego.
*/

using namespace simbo;

static const std::vector<std::string> ISO2_COUNTRY_CODES({ "", "AD", "AE", "AF", "AG", "AI", "AL", "AM", "AO", "AQ", "AR", "AS", "AT", "AU", "AW", "AX", "AZ", "BA", "BB", "BD", "BE", "BF", "BG", "BH", "BI", "BJ", "BL", "BM", "BN", "BO", "BQ", "BR", "BS", "BT", "BV", "BW", "BY", "BZ", "CA", "CC", "CD", "CF", "CG", "CH", "CI", "CK", "CL", "CM", "CN", "CO", "CR", "CU", "CV", "CW", "CX", "CY", "CZ", "DE", "DJ", "DK", "DM", "DO", "DZ", "EC", "EE", "EG", "EH", "ER", "ES", "ET", "FI", "FJ", "FK", "FM", "FO", "FR", "GA", "GB", "GD", "GE", "GF", "GG", "GH", "GI", "GL", "GM", "GN", "GP", "GQ", "GR", "GS", "GT", "GU", "GW", "GY", "HK", "HM", "HN", "HR", "HT", "HU", "ID", "IE", "IL", "IM", "IN", "IO", "IQ", "IR", "IS", "IT", "JE", "JM", "JO", "JP", "KE", "KG", "KH", "KI", "KM", "KN", "KP", "KR", "KW", "KY", "KZ", "LA", "LB", "LC", "LI", "LK", "LR", "LS", "LT", "LU", "LV", "LY", "MA", "MC", "MD", "ME", "MF", "MG", "MH", "MK", "ML", "MM", "MN", "MO", "MP", "MQ", "MR", "MS", "MT", "MU", "MV", "MW", "MX", "MY", "MZ", "NA", "NC", "NE", "NF", "NG", "NI", "NL", "NO", "NP", "NR", "NU", "NZ", "OM", "PA", "PE", "PF", "PG", "PH", "PK", "PL", "PM", "PN", "PR", "PS", "PT", "PW", "PY", "QA", "RE", "RO", "RS", "RU", "RW", "SA", "SB", "SC", "SD", "SE", "SG", "SH", "SI", "SJ", "SK", "SL", "SM", "SN", "SO", "SR", "SS", "ST", "SV", "SX", "SY", "SZ", "TC", "TD", "TF", "TG", "TH", "TJ", "TK", "TL", "TM", "TN", "TO", "TR", "TT", "TV", "TW", "TZ", "UA", "UG", "UM", "US", "UY", "UZ", "VA", "VC", "VE", "VG", "VI", "VN", "VU", "WF", "WS", "YE", "YT", "ZA", "ZM", "ZW" });

static seconds_t time_str_to_seconds(const std::string& time_str) {
	// Szybko sprawdź format czasu.
	if (!(
		time_str.size() == 20 &&
		time_str[4] == '-' &&
		time_str[7] == '-' &&
		time_str[10] == 'T' &&
		time_str[13] == ':' &&
		time_str[16] == ':' &&
		time_str[19] == 'Z'
		)) {
		throw DeserialisationError("DataPoint", time_str, "Time must be in the format YYYY-MM-DDThh:mm:ssZ");
	}
	const int year = std::stoi(time_str.substr(0, 4));
	const int month = std::stoi(time_str.substr(5, 2));
	const int day = std::stoi(time_str.substr(8, 2));
	const int hour = std::stoi(time_str.substr(11, 2));
	const int minute = std::stoi(time_str.substr(14, 2));
	const int second = std::stoi(time_str.substr(17, 2));
	const DateTime time(Date(year, month, day), TimeDuration(hour, minute, second));
	return to_seconds(time - n6::EPOCH);
}

int main(int argc, char* argv[]) {
	try {
		cxxopts::Options options("sinkhole_data_converter", "Sinkhole data converter");
		options.add_options()
			("i,input", "Path to source JSON file", cxxopts::value<std::string>())
			("o,output", "Path to output BIN file", cxxopts::value<std::string>())
			("l,level", "Log level: error, warn, info, debug or trace", cxxopts::value<std::string>(std::string("info")))
			("log_filename", "Name for log file (default: log to stdout)", cxxopts::value<std::string>())
			("c,countries", "2-letter country codes to read", cxxopts::value<std::vector<std::string>>())
			("n,names", "Bot names to read", cxxopts::value<std::vector<std::string>>())
			("new_only", "Only create new files, don't overwrite existing ones")
			("h,help", "Help");
		options.parse_positional({ "input", "output" });

		const auto options_values = options.parse(argc, argv);

		if (options_values.count("help")) {
			std::cout << options.help({ "" }) << std::endl;
			return 0;
		}

		if (!options_values.count("input")) {
			throw std::runtime_error("No input file path provided (use -h for help with parameters)");
		}
		const std::string input_filename(options_values["input"].as<std::string>());

		if (!options_values.count("output")) {
			throw std::runtime_error("No output file path provided");
		}
		const std::string output_filename(options_values["output"].as<std::string>());		

		if (options_values.count("log_filename")) {
			setup_file_logging(options_values["log_filename"].as<std::string>(), false);
		}

		std::string log_level("info");
		if (options_values.count("level")) {
			log_level = options_values["level"].as<std::string>();
		}
		if (log_level == "error") {
			spdlog::set_level(spdlog::level::err);
		} else if (log_level == "warn") {
			spdlog::set_level(spdlog::level::warn);
		} else if (log_level == "info") {
			spdlog::set_level(spdlog::level::info);
		} else if (log_level == "debug") {
			spdlog::set_level(spdlog::level::debug);
		} else if (log_level == "trace") {
			spdlog::set_level(spdlog::level::trace);
		} else if (!log_level.empty()) {
			throw std::runtime_error((boost::format("Unknown log level: %s") % log_level).str());
		}

		if (options_values.count("new_only") && file_exists(output_filename)) {
			get_logger()->info("Skipping input / output file pair {} / {} because the output file exists already", input_filename, output_filename);
		}

		std::unordered_set<std::string> countries;
		if (options_values.count("countries")) {
			const auto& countries_vec = options_values["countries"].as<std::vector<std::string>>();
			countries.insert(countries_vec.begin(), countries_vec.end());
		}
		get_logger()->debug("Selected {} countries", countries.size());

		std::unordered_set<std::string> names;
		if (options_values.count("names")) {
			const auto& names_vec = options_values["names"].as<std::vector<std::string>>();
			names.insert(names_vec.begin(), names_vec.end());
		}

		std::ifstream input_file(input_filename);
		get_logger()->info("Opened file {}", input_filename);
		json input;
		try {
			input_file >> input;
		} catch (const std::exception& e) {
			get_logger()->error("Error parsing JSON data from file {}: {}", input_filename, e.what());
			std::cerr << input_filename << ": " << e.what() << std::endl;
			return -1;
		}
		input_file.close();
		get_logger()->info("Parsed JSON data from file {}, reading {} raw records", input_filename, input.size());		

		// Zapisuj dane binarne w locie do pliku tymczasowego.
		const std::string temp_output_filename = std::tmpnam(nullptr);
		std::ofstream temp_output_file(temp_output_filename, std::ios_base::trunc | std::ios::binary | std::ios::out);
		// Tymczasowo ustaw liczbę wierszy w nagłówku 
		n6::write_header(input.size(), temp_output_file);
		get_logger()->debug("Wrote a temporary header declaring {} rows to {}", input.size(), temp_output_filename);
		size_t nbr_written_rows = 0;

		size_t nbr_records_without_country_code = 0;

		for (const json& record : input) {
			expect_object(record, "DataPoint");
			expect_key(record, "DataPoint", "category");
			expect_key(record, "DataPoint", "address");
			expect_key(record, "DataPoint", "time");
			expect_key(record, "DataPoint", "confidence");
			bool problematic_entry = false;
			if (record["category"] != "bots"
				|| !record.count("origin")
				|| record["origin"] != "sinkhole"
				|| record["confidence"] == "low"
				|| !record.count("name")
				) {
				continue;
			}			
			if (!names.empty() && !names.count(record["name"])) {
				continue;
			}

			const json& ip_address_struct = record["address"][0];

			// Znajdź kod kraju.
			int country = 0;
			if (ip_address_struct.count("cc")) {
				const std::string& cc = ip_address_struct["cc"];
				const auto it = std::lower_bound(ISO2_COUNTRY_CODES.begin(), ISO2_COUNTRY_CODES.end(), cc);
				if (it != ISO2_COUNTRY_CODES.end() && *it == cc) {
					country = static_cast<int>(std::distance(ISO2_COUNTRY_CODES.begin(), it));
				}
			}
			if (!country) {
				get_logger()->debug("Country code missing in record {}", record["id"].get<std::string>());
				++nbr_records_without_country_code;
			}
			if (!countries.empty()) {
				if (country == 0 || !countries.count(ISO2_COUNTRY_CODES[country])) {
					continue;
				}				
			}
			const IpAddress ip_address(ip_address_struct["ip"]);
			const std::string time_str = record["time"];
			const seconds_t time_sec = time_str_to_seconds(time_str);
			seconds_t until_sec = time_sec;
			unsigned int count = 1;
			if (record.count("until")) {
				until_sec = time_str_to_seconds(record["until"]);
				count = static_cast<unsigned int>(record["count"]);
			}
			const n6::data_point_t data_point(time_sec, count, until_sec, ip_address, country);
			temp_output_file.write((char*)&data_point, sizeof(n6::data_point_t));
			++nbr_written_rows;
		}

		if (nbr_records_without_country_code) {
			get_logger()->warn("{} records is missing a country code", nbr_records_without_country_code);
		}
		get_logger()->info("Parsed {} data points", nbr_written_rows);
		temp_output_file.close();

		if (input.size() != nbr_written_rows) {
			// Popraw liczbę wierszy w nagłówku.
			temp_output_file = std::ofstream(temp_output_filename, std::ios::in | std::ios::binary | std::ios::out);
			temp_output_file.seekp(0);
			n6::write_header(nbr_written_rows, temp_output_file);
			temp_output_file.close();
			get_logger()->debug("Corrected number of rows in header from {} to {}", input.size(), nbr_written_rows);
		}

		get_logger()->info("Saved data to temporary binary file {}", temp_output_filename);
		boost::filesystem::rename(temp_output_filename, output_filename);
		get_logger()->info("Moved temporary file {} to permanent output file {}", temp_output_filename, output_filename);


	} catch (const std::exception& e) {
		get_logger()->error("Fatal error: {}", e.what());
		std::cerr << e.what() << '\n';
		return -1;
	}
}