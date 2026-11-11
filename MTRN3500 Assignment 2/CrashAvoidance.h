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
    }

    void threadFunction() override;

protected:
    bool getShutdownFlag() override;

    error_state processSharedMemory() override;

    error_state processHeartbeat() override;

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    SM_VehicleControl^ SM_VehicleControl_;
};
