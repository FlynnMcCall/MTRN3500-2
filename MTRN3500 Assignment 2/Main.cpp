#include "TMM.h"

// MSVC v143 – VS 2022 C++ x64/x86 build tools (v14.33-17.3)
// C++ / CLI support for v143 build tools(14.33 - 17.3)

int main(void) {
	ThreadManagement^ threadManager = gcnew ThreadManagement();
	threadManager->threadFunction();

	Console::ReadKey();
	Console::ReadKey();

	return 0;
}