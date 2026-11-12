#include "NetworkedModule.h"

error_state NetworkedModule::setUp() {
	connect(this->address, this->port);
	return UGVModule::setUp();
}

error_state NetworkedModule::connect(String ^ hostName, int portNumber) {
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

error_state NetworkedModule::authenticate() {
	SendData = Encoding::ASCII->GetBytes(AUTHENICATION);
	Stream->Write(SendData, 0, SendData->Length);

	System::Threading::Thread::Sleep(100);
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ resp = Encoding::ASCII->GetString(ReadData, 0, len);
	if (!String::Equals(resp, "OK\n")) {
		return error_state::ERR_CONNECTION;
	}
	return error_state::SUCCESS;
}