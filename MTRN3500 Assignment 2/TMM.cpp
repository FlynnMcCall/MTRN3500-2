#include "TMM.h"
#include "LiDAR.h"
#include "GNSS.h"

#define TAG "[TMM]: "

error_state ThreadManagement::setupSharedMemory() {
	try {
		SM_TM_ = gcnew SM_ThreadManagement;
		SM_Laser_ = gcnew SM_Laser;
		SM_GPS_ = gcnew SM_GPS;
		SM_VehicleControl_ = gcnew SM_VehicleControl;
		return error_state::SUCCESS;
	}
	catch (Exception^ e) {
		Console::WriteLine(TAG + "setup failed with error: {0}", e);
		return error_state::ERR_STARTUP;
	}
}

error_state ThreadManagement::setupThreads() {
	ThreadPropertiesList = gcnew array<ThreadProperties^>{
		gcnew ThreadProperties(gcnew ThreadStart(gcnew LiDAR(SM_TM_, SM_Laser_), &LiDAR::threadFunction), true, bit_LASER, "LiDAR Thread"),
		gcnew ThreadProperties(gcnew ThreadStart(gcnew GNSS(SM_TM_, SM_GPS_), &GNSS::threadFunction), false, bit_GPS, "GNSS Thread"),
	};

	ThreadList = gcnew array<Thread^>(ThreadPropertiesList->Length);
	StopwatchList = gcnew array<Stopwatch^>(ThreadPropertiesList->Length);
	SM_TM_->ThreadBarrier = gcnew Barrier(ThreadPropertiesList->Length + 1);

	return error_state::SUCCESS;
}

error_state ThreadManagement::startThreads() {
	for (int i = 0; i < ThreadPropertiesList->Length; i++)
	{
		StopwatchList[i] = gcnew Stopwatch;
		ThreadList[i] = gcnew Thread(ThreadPropertiesList[i]->ThreadStart_);
		ThreadList[i]->Start();
	}

	SM_TM_->ThreadBarrier->SignalAndWait();

	for (int i = 0; i < ThreadList->Length; i++)
	{
		StopwatchList[i]->Start();
	}

	return error_state::SUCCESS;
}

error_state ThreadManagement::processSharedMemory() {
	return error_state::SUCCESS;
}

error_state ThreadManagement::processHeartbeat()
{
	for (int i = 0; i < ThreadList->Length; i++)
	{
		if (SM_TM_->heartbeat & ThreadPropertiesList[i]->BitID)
		{
			SM_TM_->heartbeat &= ~ThreadPropertiesList[i]->BitID;
			StopwatchList[i]->Restart();
		}

		else
		{
			if (StopwatchList[i]->ElapsedMilliseconds > CRASH_LIMIT)
			{
				if (ThreadPropertiesList[i]->Critical)
				{
					Console::WriteLine(ThreadPropertiesList[i]->ThreadName + " (critical) failure. Shutting down all threads");
					shutdownModules();
					return error_state::ERR_CRITICAL_THREAD_FAILURE;
				}

				else
				{
					Console::WriteLine(ThreadPropertiesList[i]->ThreadName + " failed...Restarting");
					ThreadList[i]->Abort();
					ThreadList[i] = gcnew Thread(ThreadPropertiesList[i]->ThreadStart_);
					SM_TM_->ThreadBarrier = gcnew Barrier(1);
					ThreadList[i]->Start();
				}
			}
		}
	}

	return error_state::SUCCESS;
}
void ThreadManagement::shutdownModules() {
	SM_TM_->shutdown = 0xFF;
}

bool ThreadManagement::getShutdownFlag() {
	return (SM_TM_->shutdown) & (this->bitmask);
}

void ThreadManagement::threadFunction()
{
	Console::WriteLine(TAG + "TMM Thread is starting");

	setupSharedMemory();
	setupThreads();
	
	startThreads();
	
	Console::WriteLine(TAG + "TMM Thread is running...\nPress q to terminate");

	while (!getShutdownFlag())
	{
		processHeartbeat();
		Thread::Sleep(100);
		if (Console::KeyAvailable && Console::ReadKey(true).Key == ConsoleKey::Q) {
			break;
		}
	}
	Console::WriteLine(TAG + "TMM Thread terminating all modules");
	shutdownModules();

	//join all threads
	for (int i = 0; i < ThreadPropertiesList->Length; i++)
	{
		ThreadList[i]->Join();
	}

	Console::WriteLine(TAG + "All modules terminated");
}