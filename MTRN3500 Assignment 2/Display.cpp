#include "Display.h"

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
	setUp();

	while (!getShutdownFlag()) {
		processSharedMemory();
		communicate();
		processHeartbeat();
		Thread::Sleep(100);
	}

	Console::WriteLine(this->name + "thread returning");
}
