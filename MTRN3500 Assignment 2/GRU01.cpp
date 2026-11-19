#include "GRU01.h"
using namespace System::Globalization;
double ToDegrees(String^ value, String^ hemisphere);
bool ChecksumOK(String^ line);

error_state GRU01::connect(String^ hostName, int portNumber) {
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

error_state GRU01::communicate() {
	return error_state::SUCCESS;
}

error_state GRU01::processSharedMemory() {
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	if (len == 0) return error_state::ERR_CONNECTION;

	String^ response = Encoding::ASCII->GetString(ReadData, 0, len);
	array<String^>^ lines = response->Split('\n');

	bool found = false;
	double lat = 0., lon = 0., height = 0.;
	for (int i = lines->Length - 2; 0 <= i; i--) {
		String^ line = lines[i]->Trim();

		if (!line->StartsWith("$") || !ChecksumOK(line)) continue;

		array<String^>^ fields = line->Substring(0, line->IndexOf('*'))->Split(',');
		
		if (!fields[0]->EndsWith("GGA") || fields->Length < 10) continue;

		if (fields[6] == "0" || fields[2]->Length == 0) continue;

		try {
			lat = ToDegrees(fields[2], fields[3]);
			lon = ToDegrees(fields[4], fields[5]);
			height = Convert::ToDouble(fields[9], CultureInfo::InvariantCulture);
			found = true;
			break;
		}
		catch (Exception^) {
			continue;
		}
	}
	if (!found) return error_state::ERR_NO_DATA;


	Monitor::Enter(SM_GRU01_->lockObject);
	SM_GRU01_->N_dec = lat;
	SM_GRU01_->E_dec = lon;
	SM_GRU01_->Height = height;
	Monitor::Exit(SM_GRU01_->lockObject);

	Console::WriteLine(
		"{0}\t| Lat: {1,11:F6}\t| Long: {2,11:F6}\t| height: {3,6:F1}m",
		name, lat, lon, height
	);

	return error_state::SUCCESS;
}




void GRU01::threadFunction() {
	error_state setup_err = setUp();
	if (setup_err != error_state::SUCCESS) return;

	while (!getShutdownFlag()) {
		Thread::Sleep(100);

		error_state mem_err = processSharedMemory();
		if (mem_err != error_state::SUCCESS &&
			mem_err != error_state::ERR_NO_DATA) 
		{
			return;
		}

		error_state hb_err = processHeartbeat();
		if (hb_err != error_state::SUCCESS) return;
	}
	Console::WriteLine(this->name + "thread returning");
}

double ToDegrees(String^ value, String^ hemisphere)
{
	double val = Double::Parse(value, CultureInfo::InvariantCulture);
	double deg = Math::Floor(val / 100.0);
	double result = deg + (val - deg * 100.0) / 60.0;
	if (hemisphere == "S" || hemisphere == "W") result = -result;
	return result;
}

bool ChecksumOK(String^ line)
{
	int star = line->IndexOf('*');
	if (!line->StartsWith("$") || star < 0 || star + 3 > line->Length) return false;

	int checksum = 0;
	for (int i = 1; i < star; i++) checksum ^= line[i];

	int expected = Convert::ToInt32(line->Substring(star + 1, 2), 16);
	return checksum == expected;
}