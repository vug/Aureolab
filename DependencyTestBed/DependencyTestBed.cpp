#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

std::shared_ptr<spdlog::logger> CreateLogger() {
	std::vector<spdlog::sink_ptr> sinks = {
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
	};
	sinks[0]->set_pattern("%^[%T.%e] %n: %v%$");
	auto logger = std::make_shared<spdlog::logger>("DEPTESTBED", begin(sinks), end(sinks));
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
	return logger;
}

int main(int argc, char* argv[]) {
	auto logger = CreateLogger();
	logger->info("Hi from Dependency Test Bed!");
	return 0;
}