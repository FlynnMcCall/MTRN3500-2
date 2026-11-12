#include "LiDAR.h"

error_state LiDAR::communicate() {

	String^ cmd = "sRN LMDscandata";
	SendData = Encoding::ASCII->GetBytes(cmd);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
	/*
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ resp = Encoding::ASCII->GetString(ReadData, 0, len);
	Console::WriteLine(this->name + "{0}\n", resp);
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
		Console::WriteLine(this->name + "Wrong number of points. got {0}, expected {1}", NumPoints, STANDARD_LASER_LENGTH);
		Monitor::Exit(SM_Laser_->lockObject);
		throw gcnew Exception(this->name + "Wrong number of points");
	}

	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		SM_Laser_->x[i] =  Convert::ToInt32(Fragments[i + 26], 16) * Math::Sin(i * 0.5 * Math::PI / 180.0);
		SM_Laser_->y[i] = -Convert::ToInt32(Fragments[i + 26], 16) * Math::Cos(i * 0.5 * Math::PI / 180.0);
	}
	Monitor::Exit(SM_Laser_->lockObject);
	return error_state::SUCCESS;
}

void LiDAR::threadFunction() {
	NetworkedModule::setUp();
	authenticate();

	while (!getShutdownFlag()) {
		communicate();
		Thread::Sleep(100);
		processSharedMemory();
		processHeartbeat();
	}
	Console::WriteLine(this->name + "thread returning");
}
