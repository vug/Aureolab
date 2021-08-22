#pragma once

/*
* Platform detection definition logic taken from TheCherno's engine series.
* Non-Windows platforms are not tested yet.
*/ 
#ifdef _WIN32 /* Windows x64/x86 */
	#ifdef _WIN64 /* Windows x64 */
	#define AL_PLATFORM_WINDOWS
#else /* Windows x86 */
	#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define AL_PLATFORM_IOS
		#error "IOS is not supported!"
	#elif TARGET_OS_MAC == 1
		#define AL_PLATFORM_MACOS
		#error "MacOS is not supported!"
	#else
		#error "Unknown Apple platform!"
#endif
#elif defined(__ANDROID__)
	#define AL_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define AL_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	#error "Unknown platform!"
#endif

enum class Platform {
	WINDOWS,
	// LINUX, MACOS, IOS, ANDROID
};

class PlatformUtils {
public:
	static Platform GetPlatform() { return platform; }
private:
#ifdef AL_PLATFORM_WINDOWS
	static const Platform platform = Platform::WINDOWS;
#endif
};
