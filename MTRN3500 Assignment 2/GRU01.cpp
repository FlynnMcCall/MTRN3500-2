#include "GRU01.h"

error_state GRU01::communicate() {
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	String^ response = Encoding::ASCII->GetString(ReadData, 0, len);
	Console::WriteLine(name + response);
	return error_state::SUCCESS;
}

error_state GRU01::processSharedMemory() {
	
	Monitor::Enter(SM_GRU01_->lockObject);
	Monitor::Exit(SM_GRU01_->lockObject);
	return error_state::SUCCESS;
}

void GRU01::printData() {
}



void GRU01::threadFunction() {
	setUp();

	while (!getShutdownFlag()) {
		Thread::Sleep(100);
		processSharedMemory();
		printData();
		processHeartbeat();
	}
	Console::WriteLine(this->name + "thread returning");
}
/*

port = gcnew SerialPort("COM3", 38400);
	port->Parity = Parity::None;
	port->DataBits = 8;
	port->StopBits = StopBits::One;

	try {
		port->Open();
		while (true) {
			String^ data = port->ReadLine();
		}
	}
	catch (Exception^ e) {}

	if (port->IsOpen) port->Close();
	return error_state::SUCCESS;

*/