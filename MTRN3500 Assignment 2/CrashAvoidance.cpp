#include "CrashAvoidance.h"


void CrashAvoidance::threadFunction() {
	Console::WriteLine(this->name + "starting...");

	Console::WriteLine(this->name + "set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	Console::WriteLine(this->name + "running...");
	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();

	while (!getShutdownFlag()) {
		processSharedMemory();
		processHeartbeat();
		Thread::Sleep(20);
	}
	Console::WriteLine(this->name + "thread returning");
}


bool CrashAvoidance::getShutdownFlag() {
	return SM_TM_->shutdown & this->bitmask;
}

error_state CrashAvoidance::processSharedMemory() {
	return error_state::SUCCESS;
}

error_state CrashAvoidance::processHeartbeat() {
	if ((SM_TM_->heartbeat & this->bitmask) == 0)
	{
		SM_TM_->heartbeat |= this->bitmask;
		Stopwatch_->Restart();
	}

	else
	{
		//check if the time elasped exceed the crash time limit
		if (Stopwatch_->ElapsedMilliseconds > CRASH_LIMIT)
		{
			SM_TM_->shutdown = 0xFF;
			Console::WriteLine(name + "TMM failure, shutting down");
			return error_state::ERR_TMM_FAILURE;
		}
	}

	return error_state::SUCCESS;
}
// */