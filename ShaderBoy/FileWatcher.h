#pragma once

#include "Core/Log.h"

#include <filesystem>
#include <thread>
#include <chrono>

class FileWatcher {
public:
	FileWatcher(const std::filesystem::path& filepath)
		: filepath(filepath) {
		Log::Info("FileWatcher watching {}...", filepath.string());
	}

	static void WatchFile(FileWatcher& fw) {
		while (fw.isRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			Log::Info("heartbeat for {}...", fw.filepath.string());
		}
	}

	void Start() {
		std::thread* fileWatcherThread = new std::thread(FileWatcher::WatchFile, std::ref(*this));
		fileWatcherThread->detach();
	}

public:
	std::filesystem::path filepath;
	bool isRunning = true;
};