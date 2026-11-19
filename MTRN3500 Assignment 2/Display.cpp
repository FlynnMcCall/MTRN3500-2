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
	error_state setup_err = setUp();
	if (setup_err != error_state::SUCCESS) return;

	while (!getShutdownFlag()) {
		error_state mem_err = processSharedMemory();
		if (mem_err != error_state::SUCCESS &&
			mem_err != error_state::ERR_NO_DATA)
		{
			return;
		}
		error_state comm_err = communicate();
		if (comm_err != error_state::SUCCESS)
		{
			return;
		}

		error_state hb_err = processHeartbeat();
		if (hb_err != error_state::SUCCESS) return;
		Thread::Sleep(100);
	}
	Console::WriteLine(this->name + "thread returning");
}