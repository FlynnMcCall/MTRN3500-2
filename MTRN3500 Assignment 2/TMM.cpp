#include "TMM.h"
#include "LiDAR.h"
#include "GNSS.h"
#include "Controller.h"
#include "VC.h"
#include "Display.h"
#include "CrashAvoidance.h"
#include "GRU01.h"


error_state ThreadManagement::setupSharedMemory() {
	try {
		SM_TM_ = gcnew SM_ThreadManagement;
		SM_Laser_ = gcnew SM_Laser;
		SM_GPS_ = gcnew SM_GPS;
		SM_GRU01_ = gcnew SM_GRU01;
		SM_VehicleControl_ = gcnew SM_VehicleControl;
		return error_state::SUCCESS;
	}
	catch (Exception^ e) {
		Console::WriteLine(this->name + "setup failed with error: {0}", e);
		return error_state::ERR_STARTUP;
	}
}

error_state ThreadManagement::setupThreads() {
	ModuleList = gcnew array<UGVModule^>{
		gcnew LiDAR(SM_TM_, SM_Laser_),
		//gcnew GNSS(SM_TM_, SM_GPS_),
		//gcnew GRU01(SM_TM_, SM_GRU01_),
		gcnew Controller(SM_TM_, SM_VehicleControl_),
		gcnew VehicleControl(SM_TM_, SM_VehicleControl_),
		//gcnew Display(SM_TM_, SM_Laser_),
		gcnew CrashAvoidance(SM_TM_, SM_Laser_,  SM_VehicleControl_),
	};

	ThreadList = gcnew array<Thread^>(ModuleList->Length);
	StopwatchList = gcnew array<Stopwatch^>(ModuleList->Length);
	SM_TM_->ThreadBarrier = gcnew Barrier(ModuleList->Length + 1);

	return error_state::SUCCESS;
}

error_state ThreadManagement::startThreads() {
	for (int i = 0; i < ModuleList->Length; i++)
	{
		Console::WriteLine(this->name + "launching {0}...", ModuleList[i]->name);
		StopwatchList[i] = gcnew Stopwatch;
		ThreadList[i] = gcnew Thread(gcnew ThreadStart(ModuleList[i], &UGVModule::threadFunction));
		ThreadList[i]->Start();
	}
	Console::WriteLine(name + "set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();
	Console::WriteLine(name + "running...");

	for (int i = 0; i < ThreadList->Length; i++)
	{
		StopwatchList[i]->Start();
	}

	return error_state::SUCCESS;
}

error_state ThreadManagement::processSharedMemory() {
	return error_state::SUCCESS;
}

error_state ThreadManagement::manageHeartbeats()
{
	for (int i = 0; i < ThreadList->Length; i++)
	{
		if (SM_TM_->heartbeat & ModuleList[i]->bitmask)
		{
			SM_TM_->heartbeat &= ~ModuleList[i]->bitmask;
			StopwatchList[i]->Restart();
		}

		else
		{
			if (StopwatchList[i]->ElapsedMilliseconds > CRASH_LIMIT)
			{
				if (ModuleList[i]->bitmask & CRITICALMASK)
				{
					Console::WriteLine(name + "{0} failure, shutting down", ModuleList[i]->name);
					SM_TM_->shutdown = 0xFF;
					return error_state::ERR_CRITICAL_THREAD_FAILURE;
				}

				else
				{
					Console::WriteLine(this->name + ModuleList[i]->name + " failed... Restarting");
					ThreadList[i]->Abort();
					ThreadList[i] = gcnew Thread(gcnew ThreadStart(ModuleList[i], &UGVModule::threadFunction));
					SM_TM_->ThreadBarrier = gcnew Barrier(1);
					ThreadList[i]->Start();
				}
			}
		}
	}

	return error_state::SUCCESS;
}

void ThreadManagement::threadFunction()
{
	Console::WriteLine(this->name + "starting...");

	setupSharedMemory();
	setupThreads();
	startThreads();
	
	Console::WriteLine(this->name + "Press q to terminate");

	while (!getShutdownFlag())
	{
		manageHeartbeats();
		Thread::Sleep(100);
		if (Console::KeyAvailable && Console::ReadKey(true).Key == ConsoleKey::Q) {
			break;
		}
	}
	Console::WriteLine(this->name + "{0} shutting down all", this->name);
	SM_TM_->shutdown = 0xFF;

	//join all threads
	for (int i = 0; i < ThreadList->Length; i++)
	{
		ThreadList[i]->Join();
	}

	Console::WriteLine(this->name + "All modules terminated");
}