#pragma once
/*****************
UGVModule.h
ALEXANDER CUNIO 2024
*****************/

/*
This file defines the base class for ALL UGV modules (TMM, Laser, GPS, Vehicle Control, Display, Controller).
Basic classes (TMM, Controller) will be derived directly from this class, whereas those that require networking
to external devices (GPS, Laser, VC, Display) will derive from the networked module class (which itself inherits from this class).
For clarification of inheritance requirements see diagram in the assignment specification.
*/

#include <iostream>
#include <SMObjects.h>
#define CRASH_LIMIT 1000

// ERROR HANDLING. Use this as the return value in your functions
enum error_state {
	SUCCESS,
	ERR_STARTUP,
	ERR_NO_DATA,
	ERR_INVALID_DATA,
	ERR_SM,
	ERR_CONNECTION,
	ERR_CRITICAL_THREAD_FAILURE,
	ERR_TMM_FAILURE
	// Define your own additional error types as needed
};

ref class UGVModule abstract
{
	public:
		virtual void threadFunction() = 0;
		property uint8_t bitmask
		{
			virtual uint8_t get() = 0;
		}
		property String^ name
		{
			virtual String^ get() = 0;
		}



	protected:
		SM_ThreadManagement^ SM_TM_;
		Stopwatch^ Stopwatch_;

		virtual error_state processSharedMemory() = 0;

		virtual error_state processHeartbeat() = 0;

		virtual bool getShutdownFlag() = 0;

		static void printError(error_state error)
		{
			switch (error)
			{
				case SUCCESS:
					std::cout << "Success." << std::endl;
					break;
				case ERR_NO_DATA:
					std::cout << "ERROR: No Data Available." << std::endl;
					break;
				case ERR_INVALID_DATA:
					std::cout << "ERROR: Invalid Data Received." << std::endl;
					break;
				// ADD PRINTOUTS FOR OTHER ERROR TYPES
			}
		}

		
};
