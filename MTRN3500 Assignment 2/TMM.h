#pragma once

#include <UGVModule.h>

ref class ThreadManagement : public UGVModule {
public:
    void threadFunction() override;

    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_TM;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[TMM] ";
        }
    }

protected:

    error_state setupSharedMemory();

    error_state setupThreads();

    error_state startThreads();

    error_state processSharedMemory() override;

    error_state manageHeartbeats();

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    SM_GPS^ SM_GPS_;
    SM_GRU01^ SM_GRU01_;
    SM_VehicleControl^ SM_VehicleControl_;

    array<Stopwatch^>^ StopwatchList;
    array<Thread^>^ ThreadList;
    array<UGVModule^>^ ModuleList;
};
