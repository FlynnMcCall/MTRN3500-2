#include "VC.h"

error_state VehicleControl::communicate() {
	w_dog = !w_dog;
	double sendspeed = TargetSpeed;
	if (0 <= Throttle && Throttle <= 1.0) {
		sendspeed = TargetSpeed * Throttle;
	}
	else {
		Console::WriteLine(name + "Invalid throttle of {0}", Throttle);
	}

	String^ text = String::Format("# {0} {1} {2} #", TargetSteer, sendspeed, (int)w_dog);
	//Console::WriteLine(text);
	SendData = Encoding::ASCII->GetBytes(text);
	Stream->Write(SendData, 0, SendData->Length);
	return error_state::SUCCESS;
}

error_state VehicleControl::processSharedMemory() {
	Monitor::Enter(SM_VehicleControl_->lockObject);
	Throttle = SM_VehicleControl_->throttle;
	TargetSteer = SM_VehicleControl_->Steering;
	TargetSpeed = SM_VehicleControl_->Speed;
	Monitor::Exit(SM_VehicleControl_->lockObject);
	
	return error_state::SUCCESS;
}

void VehicleControl::threadFunction() {
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

	TargetSteer = 0.0;
	TargetSpeed = 0.0;
	communicate();

	Console::WriteLine(this->name + "thread returning");
}