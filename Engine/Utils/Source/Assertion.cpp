#include "Assertion.h"
#include "Logger.h"

void impl_utils::assertCriticalLog(bool value, std::string message, const char* fileName, const int lineNumber) {
	if (!(value)) {
		V_LOG_R(fileName, lineNumber, message);
		//MessageBoxA(nullptr, message, "An exception has occured!", MB_ICONERROR);
		//__debugbreak();
		throw std::exception(("Critical assertion fail: " + message).c_str());
	}
}

bool impl_utils::assertFailLog(bool value, std::string message, const char* fileName, const int lineNumber) {
	if (!(value)) {
		V_LOG_R(fileName, lineNumber, message);
		return true;
	}

	return false;
}