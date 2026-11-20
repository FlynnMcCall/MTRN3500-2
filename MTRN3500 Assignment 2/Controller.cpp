#include "Controller.h"

// Xbox: 0
// Keyboard : 1
#define INPUT_MODE 1

void PrintControllerState(controllerState% s);

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
	error_state contr_setup_err = controllerSetUp();
	if (contr_setup_err != error_state::SUCCESS) return;

	error_state setup_err = setUp();
	if (setup_err != error_state::SUCCESS) return;

	while (!getShutdownFlag()) {
		error_state mem_err = processSharedMemory();
		if (mem_err != error_state::SUCCESS &&
			mem_err != error_state::ERR_NO_DATA)
		{
			return;
		}
		error_state hb_err = processHeartbeat();
		if (hb_err != error_state::SUCCESS) return;
		Thread::Sleep(10);
	}
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
	//PrintControllerState(state);
	if (!state.isConnected) {
		Console::WriteLine(this->name + "Controller disconnected, shutting down...");
		SM_TM_->shutdown |= 0xFF;
		blankCommands();
		return error_state::ERR_CONNECTION;
	}

	double Speed = 0;
	double Steering = 0;
	double eps = 1e-2;

	Steering = -state.rightThumbX * 40.0;
	Speed = state.rightTrigger - state.leftTrigger;
	//Console::WriteLine(name + "{0}, {1}, {2}", state.leftTrigger, state.rightTrigger, Speed);

	Monitor::Enter(SM_VehicleControl_->lockObject);
	SM_VehicleControl_->Speed = Speed;
	SM_VehicleControl_->Steering = Steering;
	Monitor::Exit(SM_VehicleControl_->lockObject);

	// Console::WriteLine(this->name + "\nSpeed\t{0:F2}\nSteer\t{1:F2}", Speed, Steering);
	// */
	return error_state::SUCCESS;
}

// */

void PrintControllerState(controllerState% s)
{
	Console::WriteLine("Connected: {0}", s.isConnected);

	Console::WriteLine("Left Thumb  X: {0,6:F3}  Y: {1,6:F3}", s.leftThumbX, s.leftThumbY);
	Console::WriteLine("Right Thumb X: {0,6:F3}  Y: {1,6:F3}", s.rightThumbX, s.rightThumbY);
	Console::WriteLine("Triggers    L: {0,6:F3}  R: {1,6:F3}", s.leftTrigger, s.rightTrigger);

	Console::WriteLine("Buttons  A:{0} B:{1} X:{2} Y:{3}",
		s.buttonA, s.buttonB, s.buttonX, s.buttonY);
	Console::WriteLine("D-Pad    Up:{0} Down:{1} Left:{2} Right:{3}",
		s.DpadUp, s.DpadDown, s.DpadLeft, s.DpadRight);
	Console::WriteLine("Thumbs   L:{0} R:{1}", s.leftThumb, s.rightThumb);
	Console::WriteLine("Shoulder L:{0} R:{1}", s.leftShoulder, s.rightShoulder);
	Console::WriteLine("Start:{0} Back:{1}", s.start, s.back);
}