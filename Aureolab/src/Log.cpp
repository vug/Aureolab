#include "Log.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <memory>
#include <array>

std::shared_ptr<spdlog::logger> Log::logger_ = spdlog::stdout_color_mt("AUREOLAB");

void Log::Init() {
	std::array<spdlog::sink_ptr, 2> sinks = {
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
		std::make_shared<spdlog::sinks::basic_file_sink_mt>("Aureolab.log", true),
	};
	sinks[0]->set_pattern("%^[%T.%e] %n: %v%$");  // [02:55:17.392] AUREOLAB: Hello, Aureolab!   (colored)
	sinks[1]->set_pattern("[%T.%e] [%l] %n: %v"); // [02:55:17.392] [info] AUREOLAB: Hello, Aureolab!

	logger_ = std::make_shared<spdlog::logger>("AUREOLAB", begin(sinks), end(sinks));
	logger_->set_level(spdlog::level::trace);
	logger_->flush_on(spdlog::level::trace);
}