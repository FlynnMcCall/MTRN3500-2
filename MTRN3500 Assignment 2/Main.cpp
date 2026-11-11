#include "TMM.h"

int main(void) {
	ThreadManagement^ threadManager = gcnew ThreadManagement();
	threadManager->threadFunction();

	Console::ReadKey();
	Console::ReadKey();

	return 0;
}