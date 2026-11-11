#include "LiDAR.h"

#define TAG "[LiDAR]: "

void LiDAR::setup() {
	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();
}

error_state LiDAR::connect(String^ hostName, int portNumber) {
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

	/*
	* Add extra config maybe?
	* Verify settings with [SRN LMPscancfg]
	*/
	return error_state::SUCCESS;
}

error_state LiDAR::communicate() {

	String^ cmd = "sRN LMDscandata";
	SendData = Encoding::ASCII->GetBytes(cmd);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	/*
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ resp = Encoding::ASCII->GetString(ReadData, 0, len);
	Console::WriteLine(TAG + "{0}\n", resp);
	// */

	return error_state::SUCCESS;
}

error_state LiDAR::processSharedMemory() {
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ response = Encoding::ASCII->GetString(ReadData, 0, len);

	array<String^>^ Fragments;
	Fragments = response->Split(' ');
	Monitor::Enter(SM_Laser_->lockObject);
	int NumPoints = Convert::ToInt32(Fragments[25], 16);

	if (NumPoints != STANDARD_LASER_LENGTH) {
		Console::WriteLine(TAG + "Wrong number of points. got {0}, expected {1}", NumPoints, STANDARD_LASER_LENGTH);
		throw gcnew Exception(TAG + "Wrong number of points");
	}

	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		SM_Laser_->x[i] =  Convert::ToInt32(Fragments[i + 26], 16) * Math::Sin(i * 0.5 * Math::PI / 180.0);
		SM_Laser_->y[i] = -Convert::ToInt32(Fragments[i + 26], 16) * Math::Cos(i * 0.5 * Math::PI / 180.0);
	}
	Monitor::Exit(SM_Laser_->lockObject);
	return error_state::SUCCESS;
}

void LiDAR::threadFunction() {
	setup();
	connect(WEEDER_ADDRESS, LIDAR_PORT);
	Console::WriteLine(TAG + "LiDAR set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag()) {
		communicate();
		Thread::Sleep(100);
		processSharedMemory();
		processHeartbeat();
	}
}

bool LiDAR::getShutdownFlag() {
    return (SM_TM_->shutdown) & (this->bitmask);
}

error_state LiDAR::processHeartbeat() {
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
			return error_state::ERR_TMM_FAILURE;
		}
	}

	return error_state::SUCCESS;
}
