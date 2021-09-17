#pragma once

#include "Core/Log.h"

#include <filesystem>
#include <thread>
#include <chrono>
#include <functional>

class FileWatcher {
public:
	FileWatcher(const std::filesystem::path& filepath)
		: filepath(filepath) {
		Log::Info("FileWatcher watching {}...", filepath.string());
		lastWriteTime = std::filesystem::last_write_time(filepath);
	}

	static void WatchFile(FileWatcher& fw) {
		while (fw.isRunning) {
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			if (!std::filesystem::exists(fw.filepath)) {
				return;
			}
			auto currentWritetime = std::filesystem::last_write_time(fw.filepath);
			bool hasChanged = fw.lastWriteTime != currentWritetime;
			if (hasChanged) {
				fw.lastWriteTime = currentWritetime;
				fw.callback();
			}
		}
	}

	void Start(std::function<void()> onFileChanged) {
		callback = onFileChanged;
		std::thread* fileWatcherThread = new std::thread(FileWatcher::WatchFile, std::ref(*this));
		fileWatcherThread->detach();
	}

	void Stop() {
		isRunning = false;
	}

public:
	std::filesystem::path filepath;
	std::function<void()> callback;
	std::thread* fileWatcherThread = nullptr;
	bool isRunning = true;
	std::filesystem::file_time_type lastWriteTime;
};