#include "Log.h"

int main() {
	Log::Init();
	Log::Critical("This is so critical!");
	Log::Error("Halt! Ein Error!");
	Log::Warning("I'm warning you.");
	Log::Info("Hello, Aureolab!");
	Log::Debug("There are {} many bugs in this software", 42);
	Log::Trace("Tracing...");
	return 0;
}
