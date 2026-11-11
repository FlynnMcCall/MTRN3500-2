#pragma once

#include "NetworkedModule.h"

ref class LiDAR : public NetworkedModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_LASER;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[LiDAR] ";
        }
    }


    LiDAR::LiDAR(SM_ThreadManagement^ SM_TM, SM_Laser^ SM_Laser)
    {
        SM_TM_ = SM_TM;
        SM_Laser_ = SM_Laser;
    }

    void threadFunction() override;

    error_state connect(String^ hostName, int portNumber) override;
    error_state communicate() override;

protected:
    bool getShutdownFlag() override;

    error_state processSharedMemory() override;

    error_state processHeartbeat() override;

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
};
