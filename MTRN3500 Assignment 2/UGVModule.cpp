#include "UGVModule.h"

error_state UGVModule::setUp() {
	Console::WriteLine(this->name + "waiting...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	Console::WriteLine(this->name + "running...");
	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();
	return error_state::SUCCESS;
}

bool UGVModule::getShutdownFlag() {
	return (SM_TM_->shutdown) & (this->bitmask);
}


error_state UGVModule::processHeartbeat() {
	if (Stopwatch_ == nullptr) Console::WriteLine(this->name);
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

