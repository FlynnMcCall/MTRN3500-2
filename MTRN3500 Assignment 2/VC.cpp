#include "VC.h"

error_state VehicleControl::connect(String^ hostName, int portNumber) {
	Client = gcnew TcpClient(hostName, portNumber);
	Stream = Client->GetStream();
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(512);
	ReadData = gcnew array<unsigned char>(2048);

	SendData = Encoding::ASCII->GetBytes(AUTHENICATION);
	Stream->Write(SendData, 0, SendData->Length);

	System::Threading::Thread::Sleep(100);
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ resp = Encoding::ASCII->GetString(ReadData, 0, len);
	if (!String::Equals(resp, "OK\n")) {
		return error_state::ERR_CONNECTION;
	}

	return error_state::SUCCESS;
}

error_state VehicleControl::communicate() {
	w_dog = !w_dog;
	String^ text = String::Format("{0} {1} {2}", TargetSteer * 40.0, TargetSpeed, w_dog);
	// Console::WriteLine(text);
	SendData = Encoding::ASCII->GetBytes(text);
	Stream->Write(SendData, 0, SendData->Length);
	return error_state::SUCCESS;
}

error_state VehicleControl::processSharedMemory() {
	Monitor::Enter(SM_VehicleControl_->lockObject);
	TargetSteer = SM_VehicleControl_->Steering;
	TargetSpeed = SM_VehicleControl_->Speed;
	Monitor::Exit(SM_VehicleControl_->lockObject);
	return error_state::SUCCESS;
}

void VehicleControl::threadFunction() {
	Console::WriteLine(this->name + "starting...");
	connect(WEEDER_ADDRESS, VC_PORT);

	Console::WriteLine(this->name + "set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();
	Console::WriteLine(name + "running...");

	while (!getShutdownFlag()) {
		communicate();
		Thread::Sleep(100);
		processSharedMemory();
		processHeartbeat();
	}

	Console::WriteLine(this->name + "thread returning");
}

bool VehicleControl::getShutdownFlag() {
	return (SM_TM_->shutdown) & (this->bitmask);
}

error_state VehicleControl::processHeartbeat() {
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
