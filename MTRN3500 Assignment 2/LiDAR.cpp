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
	//Console::WriteLine(name + "{0}", response);

	array<String^>^ Fragments;
	Fragments = response->Split(' ');

	if (Fragments->Length <= 26) {
		Console::WriteLine(this->name + "Response too short");
		return error_state::ERR_INVALID_DATA;
	}

	Monitor::Enter(SM_Laser_->lockObject);
	int NumPoints = Convert::ToInt32(Fragments[25], 16);

	if (NumPoints != STANDARD_LASER_LENGTH) {
		Console::WriteLine(this->name + "Wrong number of points. got {0}, expected {1}", NumPoints, STANDARD_LASER_LENGTH);
		Monitor::Exit(SM_Laser_->lockObject);
		return error_state::ERR_INVALID_DATA;
		//throw gcnew Exception(this->name + "Wrong number of points");
	}

	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		SM_Laser_->x[i] = ( (double)Convert::ToInt32(Fragments[i + 26], 16) * Math::Sin(i * 0.5 * Math::PI / 180.0)) / 1000.;
		SM_Laser_->y[i] = (-(double)Convert::ToInt32(Fragments[i + 26], 16) * Math::Cos(i * 0.5 * Math::PI / 180.0)) / 1000.;
	}
	Monitor::Exit(SM_Laser_->lockObject);
	return error_state::SUCCESS;
}
void LiDAR::threadFunction() {
	error_state setup_err = setUp();
	if (setup_err != error_state::SUCCESS) return;

	error_state auth_err = authenticate();
	if (auth_err != error_state::SUCCESS) return;

	while (!getShutdownFlag()) {
		error_state comm_err = communicate();
		if (comm_err != error_state::SUCCESS) return;

		Thread::Sleep(100);

		error_state mem_err = processSharedMemory();
		if (mem_err != error_state::SUCCESS) return;

		error_state hb_err = processHeartbeat();
		if (hb_err != error_state::SUCCESS) return;
	}

	Console::WriteLine(this->name + "thread returning");
}