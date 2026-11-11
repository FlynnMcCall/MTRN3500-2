#include "GNSS.h"

#define TAG "[GNSS]: "

void GNSS::setup() {
	Stopwatch_ = gcnew Stopwatch;
	Stopwatch_->Start();
}

error_state GNSS::connect(String^ hostName, int portNumber) {
	Client = gcnew TcpClient(hostName, portNumber);
	Stream = Client->GetStream();
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;
	Client->SendTimeout = 500;
	Client->ReceiveBufferSize = 2048;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(512);
	ReadData = gcnew array<unsigned char>(1024);
	return error_state::SUCCESS;
}

// vestigial (no commands needed)
error_state GNSS::communicate() {
	return error_state::SUCCESS;
}

error_state GNSS::processSharedMemory() {
	int len = Stream->Read(ReadData, 0, ReadData->Length);
	int start = -3;
	uint32_t head = 0;

	while (head != 0x00AA4412) {
		start++;
		head = (head << 8) + ReadData[start + 2];
		head &= 0x00ffFFff;

		if (ReadData->Length - 256 < start) {
			// non fatal error
			Console::WriteLine(TAG + "no data");
			return ERR_NO_DATA;
		}
	}
	uint8_t head_len = BitConverter::ToChar(ReadData, start + 3);

	unsigned char buffer[128];
	for (int i = 0; (i < head_len + 80) || (i < 128); i++) {
		buffer[i] = ReadData[start + i];
	}

	Monitor::Enter(SM_GPS_->lockObject);
	SM_GPS_->Northing = BitConverter::ToDouble(ReadData, start + head_len + 16);
	SM_GPS_->Easting  = BitConverter::ToDouble(ReadData, start + head_len + 24);
	SM_GPS_->Height   = BitConverter::ToDouble(ReadData, start + head_len + 32);
	SM_GPS_->CRC = BitConverter::ToInt32(ReadData, start + head_len + 80);
	SM_GPS_->Computed_CRC = CalculateBlockCRC32(head_len + 80, buffer);
	Monitor::Exit(SM_GPS_->lockObject);

	return error_state::SUCCESS;
}

void GNSS::printData() {
	Monitor::Enter(SM_GPS_->lockObject);
	Console::WriteLine(
		TAG +
		((SM_GPS_->CRC == SM_GPS_->Computed_CRC) ? "" : "INVALID ") +
		L"Data Packet"
		L"\n{0:X8} CRC (given)"
		L"\t| North  {1:F3}m"
		L"\n{2:X8} CRC (found)"
		L"\t| East   {3:F3}m"
		L"\n\t\t"
		L"\t| Height {4:F3}m",
		SM_GPS_->CRC,
		SM_GPS_->Northing,
		SM_GPS_->Computed_CRC,
		SM_GPS_->Easting,
		SM_GPS_->Height);
	Monitor::Exit(SM_GPS_->lockObject);
}

error_state GNSS::processHeartbeat() {
	if ((SM_TM_->heartbeat & this->bitmask) == 0)
	{
		SM_TM_->heartbeat |= this->bitmask;
		Stopwatch_->Restart();
	}

	else
	{
		//check if the time elasped exceed the crash time limit
		if (Stopwatch_->ElapsedMilliseconds > CRASH_LIMIT)
		{
			SM_TM_->shutdown = 0xFF;
			return error_state::ERR_TMM_FAILURE;
		}
	}
	return error_state::SUCCESS;
}



void GNSS::threadFunction() {
	setup();
	connect(WEEDER_ADDRESS, GNSS_PORT);
	Console::WriteLine(TAG + "GNSS set up...");
	SM_TM_->ThreadBarrier->SignalAndWait();

	while (!getShutdownFlag()) {
		Thread::Sleep(100);
		processSharedMemory();
		printData();
		processHeartbeat();
	}
	Console::WriteLine(TAG + "thread returning");
}

bool GNSS::getShutdownFlag() {
	return (SM_TM_->shutdown) & (this->bitmask);
}



#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1) {
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		}
		else {
			ulCRC >>= 1;
		}
	}
	return ulCRC;
}
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer)
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}