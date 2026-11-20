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
#define GRU01_ADDRESS "192.168.1.200"

#define DISPLAY_ADDRESS "127.0.0.1" // Display is always running locally (run the MATLAB code separately)

#define AUTHENICATION "5488467\n"

#define LIDAR_PORT	 23000
#define GNSS_PORT	 24000
#define VC_PORT		 25000
#define GRU01_PORT	 26000
#define DISPLAY_PORT 28000

ref class NetworkedModule abstract : public UGVModule
{
	public:
		property String^ address
		{
			virtual String^ get() = 0;
		}
		property int port
		{
			virtual int get() = 0;
		}
		virtual error_state connect(String^ hostName, int portNumber) {
			Client = gcnew TcpClient(hostName, portNumber);
			Stream = Client->GetStream();
			Client->NoDelay = true;
			Client->ReceiveTimeout = 500;
			Client->SendTimeout = 500;
			Client->ReceiveBufferSize = 2048;
			Client->SendBufferSize = 1024;

			SendData = gcnew array<unsigned char>(1024);
			ReadData = gcnew array<unsigned char>(2048);

			return error_state::SUCCESS;
		}


		virtual error_state communicate() = 0;								// Communicate over TCP connection (includes any error checking required on data)

		~NetworkedModule() {
			if (Client != nullptr) Client->Close();
			if (Stream != nullptr) Stream->Close();
		}
	protected:
		TcpClient^ Client;
		NetworkStream^ Stream;
		array<unsigned char>^ SendData;
		array<unsigned char>^ ReadData;	

		error_state setUp() override {
			connect(this->address, this->port);
			return UGVModule::setUp();
		}

		error_state authenticate() {
			SendData = Encoding::ASCII->GetBytes(AUTHENICATION);
			Stream->Write(SendData, 0, SendData->Length);

			System::Threading::Thread::Sleep(100);
			try {
				int len = Stream->Read(ReadData, 0, ReadData->Length);
				String^ resp = Encoding::ASCII->GetString(ReadData, 0, len);
				if (!String::Equals(resp, "OK\n")) {
					return error_state::ERR_CONNECTION;
				}
				return error_state::SUCCESS;
			}
			catch (Exception^ e) {
				return error_state::ERR_CONNECTION;
			}
		}
};
