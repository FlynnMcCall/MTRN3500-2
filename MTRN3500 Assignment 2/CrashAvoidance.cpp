#include "CrashAvoidance.h"

void CrashAvoidance::threadFunction() {
	setUp();

	while (!getShutdownFlag()) {
		processSharedMemory();
		processHeartbeat();
		Thread::Sleep(20);
	}
	Console::WriteLine(this->name + "thread returning");
}



error_state CrashAvoidance::processSharedMemory() {
	return error_state::SUCCESS;
}
