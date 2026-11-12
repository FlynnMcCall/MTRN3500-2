#include "Controller.h"

// Xbox: 0
// Keyboard : 1
#define INPUT_MODE 1

error_state Controller::controllerSetUp() {
	Console::WriteLine(this->name + "connecting...");

	ControllerInterface_ = new ControllerInterface(1, INPUT_MODE);
	int i = 0;
	while (!ControllerInterface_->IsConnected()) {
		Thread::Sleep(50);
		if (i > 100) {
			Console::WriteLine(this->name + "failed connection; aborting");
			delete ControllerInterface_;
			SM_TM_->ThreadBarrier->SignalAndWait();
			SM_TM_->shutdown |= 0xFF;
			return error_state::ERR_CONNECTION;
		}
	}

	String^ mode = (INPUT_MODE == 0) ? "Xbox" : "keyboard";
	Console::WriteLine(this->name + "mode {0}", INPUT_MODE);
}

void Controller::threadFunction() {
	controllerSetUp();
	setUp();

	while (!getShutdownFlag()) {
		processSharedMemory();
		processHeartbeat();
		Thread::Sleep(10);
	}
	delete ControllerInterface_;
	blankCommands();
	Console::WriteLine(this->name + "thread returning");
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
	if (INPUT_MODE == 0 &&  state.back) {
		SM_TM_->shutdown |= 0xFF;
		Console::WriteLine(this->name + "shutdown button pressed, shutting down...");
	}
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

	// Console::WriteLine(this->name + "\nSpeed\t{0:F2}\nSteer\t{1:F2}", Speed, Steering);
	// */
	return error_state::SUCCESS;
}

// */