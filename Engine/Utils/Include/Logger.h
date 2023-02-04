#pragma once
#include <DxtkString.h>

#define DEBUG_RELEASE

#if !defined(_DEBUG) && !defined(DEBUG_RELEASE)
template<typename T>
std::string format(T arg) {
	return "";
}

template <typename T, typename... Args>
std::string format(T arg, Args... args) {
	return "";
}

#define CLOG(...) ::OutputDebugString(format(__VA_ARGS__).c_str())

#define LOG_A(...)
#define LOG_B(...)
#define LOG_C(...)
#define LOG_D(...)
#define LOG_W(...)
#define LOG_R(...)
#define LOG_I(...)

#define V_LOG_A(...)
#define V_LOG_B(...)
#define V_LOG_C(...)
#define V_LOG_D(...)
#define V_LOG_W(...)
#define V_LOG_R(...)
#define V_LOG_I(...)

#else
#include <sstream>

template<typename T>
std::string format(T arg) {
	std::stringstream ss;
	ss << arg << " | ";
	return ss.str();
}

template <typename T, typename... Args>
std::string format(T arg, Args... args) {
	std::stringstream ss;
	ss << arg << " | ";
	ss << format(args...);
	return ss.str();
}

#define CLOG(...) LOG_CONSOLE(__VA_ARGS__); ::OutputDebugString(format(__VA_ARGS__).c_str())

#define LOG_A(...) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_B(...) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_C(...) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_D(...) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_W(...) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_R(...) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_I(...) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)
#define LOG_CONSOLE(...) V_LOG_CONSOLE(INTERNAL_FILE_, INTERNAL_LINE_, __VA_ARGS__)

#define V_LOG_A (VLog::getInstance().getCustomLogA())
#define V_LOG_B (VLog::getInstance().getCustomLogB())
#define V_LOG_C (VLog::getInstance().getCustomLogC())
#define V_LOG_D (VLog::getInstance().getCustomLogD())
#define V_LOG_W (VLog::getInstance().getWarningLog())
#define V_LOG_R (VLog::getInstance().getErrorLog())
#define V_LOG_I (VLog::getInstance().getInfoLog())
#define V_LOG_CONSOLE (VLog::getInstance().getConsoleLog())

#define INTERNAL_FILE_ VLog::getRelativeFileName(__FILE__)
#define INTERNAL_LINE_ VLog::getLine(__LINE__)
#endif

#include <fstream>
class LoggerInit
{
public:
	LoggerInit();

	std::fstream logFile;
};

class Logger
{
public:
	Logger();
	Logger(std::string type, LoggerInit* tmpLogFile);

	template<class ...T>
	void operator() (T... args) {
		temporaryLogFile_->logFile << type_;
		logHelper(args...);
		temporaryLogFile_->logFile << "\n";
	}

	template<class T>
	Logger& operator << (T arg) {
		temporaryLogFile_->logFile << type_ << arg << "\n";
		return *this;
	}

private:
	template<class DT, class ...T>
	void logHelper(DT arg, T... args) {
		temporaryLogFile_->logFile << arg << "|";
		logHelper(args...);
	}

	template<class DT>
	void logHelper(DT arg) {
		temporaryLogFile_->logFile << arg;
	}

	std::string type_;
	LoggerInit* temporaryLogFile_;
};

class VLog
{
public:
	VLog(VLog const&) = delete;
	void operator=(VLog const&) = delete;

	static VLog& getInstance();
	static std::string getLine(int lineNr);
	static std::string getRelativeFileName(std::string fullPath);

	Logger& getCustomLogA() const { return *logA_; }
	Logger& getCustomLogB() const { return *logB_; }
	Logger& getCustomLogC() const { return *logC_; }
	Logger& getCustomLogD() const { return *logD_; }
	Logger& getWarningLog() const { return *logW_; }
	Logger& getErrorLog() const { return *logR_; }
	Logger& getInfoLog() const { return *logI_; }
	Logger& getConsoleLog() const { return *logConsole_; }

private:
	VLog();

	Logger* logA_;
	Logger* logB_;
	Logger* logC_;
	Logger* logD_;
	Logger* logW_;
	Logger* logR_;
	Logger* logI_;
	Logger* logConsole_;
};
