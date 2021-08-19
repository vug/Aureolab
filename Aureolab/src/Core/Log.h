#pragma once
#include <spdlog/spdlog.h>
//#include <../../spdlog-src/include/spdlog/spdlog.h>
//#include <../../out/build/x64-Debug/_deps/spdlog-src/include/spdlog/spdlog.h>

#include <memory>

class Log {
public:
	Log() = delete;

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
	static std::shared_ptr<spdlog::logger> Initialize();
	static std::shared_ptr<spdlog::logger> logger_;
};