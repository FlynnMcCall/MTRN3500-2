#include "GRU01.h"

// vestigial (no commands needed)
error_state GRU01::communicate() {
	return error_state::SUCCESS;
}

error_state GRU01::processSharedMemory() {
	
	Monitor::Enter(SM_GRU01_->lockObject);
	SM_GRU01_->Northing = 0.;
	SM_GRU01_->Easting = 0.;
	SM_GRU01_->Height = 0.;
	SM_GRU01_->CRC = 0;
	SM_GRU01_->Computed_CRC = 0;
	Monitor::Exit(SM_GRU01_->lockObject);

	return error_state::SUCCESS;
}

void GRU01::printData() {
	Monitor::Enter(SM_GRU01_->lockObject);
	Console::WriteLine(
		this->name +
		((SM_GRU01_->CRC == SM_GRU01_->Computed_CRC) ? "" : "INVALID ") +
		L"Data Packet"
		L"\n{0:X8} CRC (given)"
		L"\t| North  {1:F3}m"
		L"\n{2:X8} CRC (found)"
		L"\t| East   {3:F3}m"
		L"\n\t\t"
		L"\t| Height {4:F3}m",
		SM_GRU01_->CRC,
		SM_GRU01_->Northing,
		SM_GRU01_->Computed_CRC,
		SM_GRU01_->Easting,
		SM_GRU01_->Height);
	Monitor::Exit(SM_GRU01_->lockObject);
}



void GRU01::threadFunction() {
	setUp();

	while (!getShutdownFlag()) {
		Thread::Sleep(100);
		processSharedMemory();
		printData();
		processHeartbeat();
	}
	Console::WriteLine(this->name + "thread returning");
}
