#include "VC.h"

error_state VehicleControl::communicate() {
	w_dog = !w_dog;
	double sendspeed = Freeze ? 0.0 : TargetSpeed;
	String^ text = String::Format("# {0} {1} {2} #", TargetSteer * 40.0, sendspeed, w_dog);
	//Console::WriteLine(text);
	SendData = Encoding::ASCII->GetBytes(text);
	Stream->Write(SendData, 0, SendData->Length);
	return error_state::SUCCESS;
}

error_state VehicleControl::processSharedMemory() {
	Monitor::Enter(SM_VehicleControl_->lockObject);
	Freeze = SM_VehicleControl_->freeze;
	TargetSteer = SM_VehicleControl_->Steering;
	TargetSpeed = SM_VehicleControl_->Speed;
	Monitor::Exit(SM_VehicleControl_->lockObject);
	
	return error_state::SUCCESS;
}

void VehicleControl::threadFunction() {
	setUp();
	authenticate();

	while (!getShutdownFlag()) {
		communicate();
		Thread::Sleep(100);
		processSharedMemory();
		processHeartbeat();
	}

	TargetSteer = 0.0;
	TargetSpeed = 0.0;
	communicate();

	Console::WriteLine(this->name + "thread returning");
}