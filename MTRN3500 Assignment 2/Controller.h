#pragma once
#include "ControllerInterface.h"
#include "UGVModule.h"

ref class Controller : public UGVModule {
public:
    Controller(SM_ThreadManagement^ SM_TM, SM_VehicleControl^ SM_VehicleControl) {
        SM_TM_ = SM_TM;
        SM_VehicleControl_ = SM_VehicleControl;
    }

    void threadFunction() override;

    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_CONTROLLER;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[Controller] ";
        }
    }


protected:

    bool getShutdownFlag() override;

    error_state processSharedMemory() override;

    error_state processHeartbeat() override;

    void blankCommands();

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_VehicleControl^ SM_VehicleControl_;
    ControllerInterface* ControllerInterface_ = nullptr;
};
