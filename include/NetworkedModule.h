#pragma once
/*****************
NetworkedModule.h
ALEXANDER CUNIO 2024
*****************/

/*
This class provides additional functionality for networked modules (Laser, GPS, Vehicle control, Display) beyond the
base UGV module class. These modules should therefore be derived from this rather than the base class.
For clarification of inheritance requirements see diagram in the assignment specification.
*/

#include <UGVModule.h>

// You will need to select which address to use depending on if you are working with the simulator (127.0.0.1)
// or the physical robot in the lab (192.168.1.200).
#define WEEDER_ADDRESS "127.0.0.1"
//#define WEEDER_ADDRESS "192.168.1.200"
#define DISPLAY_ADDRESS "127.0.0.1" // Display is always running locally (run the MATLAB code separately)

#define AUTHENICATION "5488467\n"

#define LIDAR_PORT 23000
#define GNSS_PORT  24000
#define VC_PORT	   25000

ref class NetworkedModule abstract : public UGVModule
{
	public:
		virtual error_state connect(String^ hostName, int portNumber) = 0;	// Establish TCP connection
		virtual error_state communicate() = 0;								// Communicate over TCP connection (includes any error checking required on data)

	protected:
		TcpClient^ Client;
		NetworkStream^ Stream;
		array<unsigned char>^ SendData;
		array<unsigned char>^ ReadData;	
};
