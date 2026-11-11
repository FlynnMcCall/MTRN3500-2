#include "Display.h"

error_state Display::connect(String^ hostName, int portNumber) {
	Client = gcnew TcpClient(hostName, portNumber);
	Stream = Client->GetStream();
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(512);
	ReadData = gcnew array<unsigned char>(2048);

	dataX = gcnew array<Byte>(STANDARD_LASER_LENGTH * sizeof(double));
	dataY = gcnew array<Byte>(STANDARD_LASER_LENGTH * sizeof(double));

	return error_state::SUCCESS;
}


error_state Display::communicate() {
	Stream->Write(dataX, 0, dataX->Length);
	Thread::Sleep(10);
	Stream->Write(dataY, 0, dataY->Length);
	return error_state::SUCCESS;
}

error_state Display::processSharedMemory() {
	Monitor::Enter(SM_Laser_->lockObject);
	Buffer::BlockCopy(SM_Laser_->x, 0, dataX, 0, dataX->Length);
	Buffer::BlockCopy(SM_Laser_->y, 0, dataY, 0, dataY->Length);
	Monitor::Exit(SM_Laser_->lockObject);
	return error_state::SUCCESS;
}

void Display::threadFunction() {
	Console::WriteLine(this->name + "starting...");
	connect(WEEDER_ADDRESS, VC_PORT);

	Console::WriteLine(this->name + "set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();
	Console::WriteLine(name + "running...");

	while (!getShutdownFlag()) {
		processSharedMemory();
		communicate();
		processHeartbeat();
		Thread::Sleep(100);
	}

	Console::WriteLine(this->name + "thread returning");
}

bool Display::getShutdownFlag() {
	return (SM_TM_->shutdown) & (this->bitmask);
}

error_state Display::processHeartbeat() {
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

