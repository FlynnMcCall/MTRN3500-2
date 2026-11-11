#pragma once

#include <UGVModule.h>

ref struct ThreadProperties
{
    ThreadStart^ ThreadStart_;
    bool Critical;
    String^ ThreadName;
    uint8_t BitID;

    ThreadProperties(ThreadStart^ start, bool critical, uint8_t bit_id, String^ threadname)
    {
        ThreadStart_ = start;
        Critical = critical;
        BitID = bit_id;
        ThreadName = threadname;
    }
};

ref class ThreadManagement : public UGVModule {
public:
    void threadFunction() override;

    void shutdownModules();

    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_TM;
        }
    }

protected:

    error_state setupSharedMemory();

    error_state setupThreads();

    error_state startThreads();

    bool getShutdownFlag() override;

    error_state processSharedMemory() override;

    error_state processHeartbeat() override;

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_Laser^ SM_Laser_;
    SM_GPS^ SM_GPS_;
    SM_VehicleControl^ SM_VehicleControl_;

    array<Stopwatch^>^ StopwatchList;
    array<Thread^>^ ThreadList;
    array<ThreadProperties^>^ ThreadPropertiesList;
};
