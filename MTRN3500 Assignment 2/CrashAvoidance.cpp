#include "CrashAvoidance.h"
#include <cmath>
void rotateInPlace(double* x, double* y, double x_center, double y_center, double rads);

void CrashAvoidance::threadFunction() {
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
		Thread::Sleep(100);
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
	Monitor::Exit(SM_VehicleControl_->lockObject);

	return error_state::SUCCESS;
}

error_state CrashAvoidance::processSharedMemory() {
	copyData();

	// checkCollisions may take a while -> compute in non-blocking way
	double  d_crash = INFINITY;
	if (Math::Abs(TargetSteer) < use_rect_deg) {
		d_crash = rectCollision();
	}
	else {
		d_crash = rotoCollision();
	}

	double throttle = findThrottle(d_crash);

	Monitor::Enter(SM_VehicleControl_->lockObject);
	SM_VehicleControl_->throttle = throttle;
	Monitor::Exit(SM_VehicleControl_->lockObject);

	return error_state::SUCCESS;
}

double CrashAvoidance::rectCollision() {
	double min_dist = INFINITY;
	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		double x = dataX[i] + LidarOffset;
		double y = dataY[i];
		if (y < -vehicleWidth / 2. || vehicleWidth / 2. < y) continue;
		min_dist = Math::Min(x, min_dist);
	}
	return min_dist;
}

double CrashAvoidance::rotoCollision(){
	double min_dist = INFINITY;
	double R = vehicleLength / Math::Tan(TargetSteer * Math::PI / 180.);

	double lowerB = Math::Sqrt(vehicleLength * vehicleLength + (R - vehicleWidth / 2.0) * (R - vehicleWidth / 2.0));
	double upperB = Math::Sqrt(vehicleLength * vehicleLength + (R + vehicleWidth / 2.0) * (R + vehicleWidth / 2.0));

	for (int i = 0; i < STANDARD_LASER_LENGTH; i++) {
		double x = dataX[i] + LidarOffset;
		double y = dataY[i];
		if (TargetSteer < 0.0) y = -y; // mirror for neg steer

		x = x + (-vehicleLength);
		y = y + (R);

		double P_mag = Math::Sqrt(x * x + y * y);
		if (P_mag < lowerB || upperB < P_mag) continue;

		// finish later
	}
	return min_dist;
}

double CrashAvoidance::findThrottle(double d_crash) {
	return (d_crash - d_min) / (d_throttle - d_min);
}

void rotateInPlace(double* x, double* y, double x_center, double y_center, double rads) {
	double netx = *x - x_center;
	double nety = *y - y_center;
	*x = netx * Math::Cos(rads) + nety * Math::Sin(rads) + x_center;
	*y = -nety * Math::Sin(rads) + netx * Math::Cos(rads) + y_center;
}