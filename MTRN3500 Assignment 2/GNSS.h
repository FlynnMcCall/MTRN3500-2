#pragma once
#include "NetworkedModule.h"

ref class GNSS : public NetworkedModule {
public:
    property uint8_t bitmask
    {
        uint8_t get() override {
            return bit_GPS;
        }
    }
    property String^ name
    {
        String^ get() override {
            return "[GNSS] ";
        }
    }


    GNSS::GNSS(SM_ThreadManagement^ SM_TM, SM_GPS^ SM_GPS)
    {
        SM_TM_ = SM_TM;
        SM_GPS_ = SM_GPS;
    }

    void threadFunction() override;

    error_state connect(String^ hostName, int portNumber) override;
    error_state communicate() override;

protected:
    bool getShutdownFlag() override;

    error_state processSharedMemory() override;

    error_state processHeartbeat() override;

    void setup();

    void printData();

private:
    // Add any additional data members or helper functions here
    // SM_ThreadManagement^ SM_TM_;
    SM_GPS^ SM_GPS_;
};

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);