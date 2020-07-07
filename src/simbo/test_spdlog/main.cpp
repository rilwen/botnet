#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

int main() {
	spdlog::set_level(spdlog::level::debug);
	auto logger = spdlog::stdout_logger_st("stdout");
	spdlog::get("stdout")->debug("Log entry");
}
