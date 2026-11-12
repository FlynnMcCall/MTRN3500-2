#pragma once

#include "NetworkedModule.h"

ref class VehicleControl : public NetworkedModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_VC;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[VehicleControl] ";
        }
    }
    property int port
    {
        int get() override {
            return VC_PORT;
        }
    }


    VehicleControl::VehicleControl(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl)
    {
        SM_TM_ = SM_TM;
        SM_VehicleControl_ = SM_VehicleControl;
    }

    void threadFunction() override;
    error_state communicate() override;

protected:
    error_state processSharedMemory() override;

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    double TargetSteer = 0;
    double TargetSpeed = 0;
    bool w_dog = true;
    SM_VehicleControl^ SM_VehicleControl_;
};
