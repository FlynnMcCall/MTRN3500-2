#pragma once

#include "UGVModule.h"

ref class CrashAvoidance : public UGVModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_CRASHAVOIDANCE;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[CrashAvoidance] ";
        }
    }


    CrashAvoidance::CrashAvoidance(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser, SM_VehicleControl^ SM_VehicleControl)
    {
        SM_TM_ = SM_TM;
        SM_Laser_ = SM_Laser;
        SM_VehicleControl_ = SM_VehicleControl;
        dataX = gcnew array<double>(STANDARD_LASER_LENGTH);
        dataY = gcnew array<double>(STANDARD_LASER_LENGTH);
    }

    void threadFunction() override;

protected:
    error_state processSharedMemory() override;

    // return true if collision predicted
    bool checkCollisions();

    bool CrashAvoidance::lookAhead(double radius, double lookahead_r);
    error_state copyData();

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    SM_VehicleControl^ SM_VehicleControl_;
    
    double nogo_front = 1.0;  // how far forwards the nogo zone protrudes (m)
    double vehicleWidth = 1.0;
    double vehicleLength = 1.0; // axel-to-axel length of vehicle

    double secondsLookahead = 1.0; // how many seconds we look ahead
    int numIts = 10; // num its

    bool Freeze = false;
    double TargetSteer = 0;
    double TargetSpeed = 0;

    array<double>^ dataX;
    array<double>^ dataY;
};
