﻿#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include "Aureolab.h"

int main() {
	auto console = spdlog::stdout_color_mt("console");
	console->info("Hello, Aureolab!");
	return 0;
}
