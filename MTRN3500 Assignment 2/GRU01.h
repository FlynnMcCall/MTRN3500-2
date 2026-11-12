#pragma once
#include "NetworkedModule.h"

ref class GRU01 : public NetworkedModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_GRU01;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[GRU01] ";
        }
    }
    property String^ address
    {
        String^ get() override {
            return GRU01_ADDRESS;
        }
    }
    property int port
    {
        int get() override {
            return GRU01_PORT;
        }
    }


    GRU01::GRU01(SM_ThreadManagement^ SM_TM, SM_GRU01^ SM_GRU01)
    {
        SM_TM_ = SM_TM;
        SM_GRU01_ = SM_GRU01;
    }

    void threadFunction() override;
    error_state communicate() override;
    // error_state connect() override;

protected:
    error_state processSharedMemory() override;

    void printData();

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_GRU01^ SM_GRU01_;
};
