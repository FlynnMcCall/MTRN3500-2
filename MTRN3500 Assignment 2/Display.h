#pragma once

#include "NetworkedModule.h"

ref class Display : public NetworkedModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_DISPLAY;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[Display] ";
        }
    }
    property int port
    {
        int get() override {
            return VC_PORT;
        }
    }


    Display::Display(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser)
    {
        SM_TM_ = SM_TM;
        SM_Laser_ = SM_Laser;
    }

    void threadFunction() override;
    error_state communicate() override;

protected:
    error_state processSharedMemory() override;

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    array<Byte>^ dataX;
    array<Byte>^ dataY;
};
