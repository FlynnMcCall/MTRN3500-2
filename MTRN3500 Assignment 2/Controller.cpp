#include "Controller.h"

// Xbox: 0
// Keyboard : 1
#define INPUT_MODE 1

void Controller::threadFunction() {
	ControllerInterface_ = new ControllerInterface(1, INPUT_MODE);
	while (!ControllerInterface_->IsConnected()) {
		Thread::Sleep(50);
	}

	String^ mode = (INPUT_MODE == 0) ? "Xbox" : "keyboard";
	Console::WriteLine(this->name + "Controller set up for {0}...", INPUT_MODE);
	SM_TM_->ThreadBarrier->SignalAndWait();

	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();

	while (!getShutdownFlag()) {
		processSharedMemory();
		processHeartbeat();
		Thread::Sleep(10);
	}
	delete ControllerInterface_;
	blankCommands();
	Console::WriteLine(this->name + "terminated");
}


bool Controller::getShutdownFlag() {
	return SM_TM_->shutdown & this->bitmask;
}
void Controller::blankCommands() {
	Monitor::Enter(SM_VehicleControl_->lockObject);
	SM_VehicleControl_->Speed = 0.0;
	SM_VehicleControl_->Steering = 0.0;
	Monitor::Exit(SM_VehicleControl_->lockObject);
}

error_state Controller::processSharedMemory() {
	_controllerState state = ControllerInterface_->GetState();
	if (!state.isConnected) {
		Console::WriteLine(this->name + "Controller disconnected, shutting down...");
		SM_TM_->shutdown |= 0xFF;
		blankCommands();
		return error_state::ERR_CONNECTION;
	}

	double Speed = 0;
	double Steering = 0;
	double eps = 1e-2;

	// Xbox
	// controls are weird -> check in lab
	if (INPUT_MODE == 0 ) {
		Steering = state.rightThumbX;
		Speed = state.rightTrigger - state.leftTrigger;
		if (eps < Math::Abs(state.leftTrigger) && eps < Math::Abs(state.rightTrigger)) {
			Speed = 0.0;
		}
	}

	// keyboard
	if (INPUT_MODE == 1) {
		Steering = state.rightThumbX;
		Speed = state.rightTrigger - state.leftTrigger;
		if (eps < Math::Abs(state.leftTrigger) && eps < Math::Abs(state.rightTrigger)) {
			Speed = 0.0;
		}
	}
	Monitor::Enter(SM_VehicleControl_->lockObject);
	SM_VehicleControl_->Speed = Speed;
	SM_VehicleControl_->Steering = Steering;
	Monitor::Exit(SM_VehicleControl_->lockObject);

	Console::WriteLine(this->name + "\nSpeed\t{0:F2}\nSteer\t{1:F2}", Speed, Steering);
	// */
	return error_state::SUCCESS;
}

error_state Controller::processHeartbeat() {
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
// */