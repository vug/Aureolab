#pragma once
#include <spdlog/spdlog.h>

class Log {
public:
	// Initialize the Logger before using it
	static void Init();

	template<typename... Args>
	static void Critical(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->critical(fmt, args...);
	}

	template<typename... Args>
	static void Error(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->error(fmt, args...);
	}

	template<typename... Args>
	static void Warning(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->warn(fmt, args...);
	}

	template<typename... Args>
	static void Info(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->info(fmt, args...);
	}

	template<typename... Args>
	static void Debug(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->debug(fmt, args...);
	}

	template<typename... Args>
	static void Trace(fmt::format_string<Args...> fmt, Args &&...args) {
		logger_->trace(fmt, args...);
	}

private:
	static std::shared_ptr<spdlog::logger> logger_;
};