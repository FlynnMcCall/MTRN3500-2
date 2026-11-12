#include "CrashAvoidance.h"
void rotateInPlace(double* x, double* y, double x_center, double y_center, double rads);

void CrashAvoidance::threadFunction() {
	setUp();

	while (!getShutdownFlag()) {
		processSharedMemory();
		processHeartbeat();
		Thread::Sleep(60);
	}
	Console::WriteLine(this->name + "thread returning");
}


error_state CrashAvoidance::copyData() {
	Monitor::Enter(SM_Laser_->lockObject);
	Buffer::BlockCopy(SM_Laser_->x, 0, dataX, 0, dataX->Length);
	Buffer::BlockCopy(SM_Laser_->y, 0, dataY, 0, dataY->Length);
	Monitor::Exit(SM_Laser_->lockObject);

	Monitor::Enter(SM_VehicleControl_->lockObject);
	TargetSteer = SM_VehicleControl_->Steering;
	TargetSpeed = SM_VehicleControl_->Speed;
	Monitor::Exit(SM_VehicleControl_->lockObject);

	return error_state::SUCCESS;
}

error_state CrashAvoidance::processSharedMemory() {
	copyData();

	// checkCollisions may take a while -> compute in non-blocking way
	Freeze = checkCollisions();

	Monitor::Enter(SM_VehicleControl_->lockObject);
	SM_VehicleControl_->freeze = Freeze;
	Monitor::Exit(SM_VehicleControl_->lockObject);

	return error_state::SUCCESS;
}

// modifies dataX and dataY in-place
// true if crash predicted
bool CrashAvoidance::lookAhead(double radius, double lookahead_r) {
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		double prevX = dataX[i];
		double prevY = dataY[i];
		pin_ptr<double> x = &dataX[i];
		pin_ptr<double> y = &dataY[i];
		double y_center = radius * Math::Sign(TargetSteer);
		rotateInPlace((double*)x, (double*)y, -vehicleLength,  y_center, lookahead_r);

		// find y intercept
		double y_int = (prevX * (*y) - (*x) * prevY) / ((*x) - prevX);

		// check intercept in range
		if (Math::Abs(y_int) < vehicleWidth) {
			return true;
		}
	}
	return false;
}

bool CrashAvoidance::checkCollisions() {
	// don't calculate collisions if reversing
	if (TargetSpeed <= 0.0) return false;
	return true;

	// check if any in no-go-zone
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		if (dataX[i] < nogo_front && Math::Abs(dataY[i]) < vehicleWidth / 2.) {
			return true;
		}
	}

	// find turn radius from ackerman
	// radius of front wheel trace
	double radius = vehicleLength / (Math::Sin(TargetSteer * Math::PI / 180.0) + 1e-4);

	// lookahead in m and r per iteration
	double lookahead_m = secondsLookahead * TargetSpeed / (double)numIts;;
	double lookahead_r = lookahead_m / (radius * 2.0 * Math::PI);

	for (int i = 0; i < numIts; i++) {
		if (lookAhead(radius, lookahead_r)) return true;
	}

	return false;
}

void rotateInPlace(double* x, double* y, double x_center, double y_center, double rads) {
	double netx = *x - x_center;
	double nety = *y - y_center;
	*x = netx * Math::Cos(rads) + nety * Math::Sin(rads) + x_center;
	*y = -nety * Math::Sin(rads) + netx * Math::Cos(rads) + y_center;
}