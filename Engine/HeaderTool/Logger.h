/*
 * Author: Mariusz Sielicki, Stanislaw Morawski
 */

#pragma once
#include <string>

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

#include <fstream>
#include <ctime>
#include <sstream>
#include <chrono>
#include "windows.h"

template<typename T>
std::string format(T arg) {
	std::stringstream ss;
	ss << arg;
	return ss.str();
}

template <typename T, typename... Args>
std::string format(T arg, Args... args) {
	std::stringstream ss;
	ss << arg;
	ss << format(args...);
	return ss.str();
}


/**
 * \brief Get well-formatted first logger (up to 8 string-convertable parameters)
 * \remarks LogA("a", 15, 3.0);
 */
#define LOG_A(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogA8, INTERNAL_LogA7, INTERNAL_LogA6, INTERNAL_LogA5, INTERNAL_LogA4, INTERNAL_LogA3, INTERNAL_LogA2, INTERNAL_LogA1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted second logger (up to 8 string-convertable parameters)
 * \remarks LogB("a", 15, 3.0);
 */
#define LOG_B(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogB8, INTERNAL_LogB7, INTERNAL_LogB6, INTERNAL_LogB5, INTERNAL_LogB4, INTERNAL_LogB3, INTERNAL_LogB2, INTERNAL_LogB1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted third logger (up to 8 string-convertable parameters)
 * \remarks LogC("a", 15, 3.0);
 */
#define LOG_C(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogC8, INTERNAL_LogC7, INTERNAL_LogC6, INTERNAL_LogC5, INTERNAL_LogC4, INTERNAL_LogC3, INTERNAL_LogC2, INTERNAL_LogC1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted fourth logger (up to 8 string-convertable parameters)
 * \remarks LogD("a", 15, 3.0);
 */
#define LOG_D(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogD8, INTERNAL_LogD7, INTERNAL_LogD6, INTERNAL_LogD5, INTERNAL_LogD4, INTERNAL_LogD3, INTERNAL_LogD2, INTERNAL_LogD1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted warning logger (up to 8 string-convertable parameters)
 * \remarks LogW("a", 15, 3.0);
 */
#define LOG_W(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogW8, INTERNAL_LogW7, INTERNAL_LogW6, INTERNAL_LogW5, INTERNAL_LogW4, INTERNAL_LogW3, INTERNAL_LogW2, INTERNAL_LogW1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted error logger (up to 8 string-convertable parameters)
 * \remarks LogR("a", 15, 3.0);
 */
#define LOG_R(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogR8, INTERNAL_LogR7, INTERNAL_LogR6, INTERNAL_LogR5, INTERNAL_LogR4, INTERNAL_LogR3, INTERNAL_LogR2, INTERNAL_LogR1)(__VA_ARGS__))

 /**
 * \brief Get well-formatted info logger (up to 8 string-convertable parameters)
 * \remarks LogR("a", 15, 3.0);
 */
#define LOG_I(...) INTERNAL_expand(INTERNAL_get_check_macro_8(__VA_ARGS__, INTERNAL_LogI8, INTERNAL_LogI7, INTERNAL_LogI6, INTERNAL_LogI5, INTERNAL_LogI4, INTERNAL_LogI3, INTERNAL_LogI2, INTERNAL_LogI1)(__VA_ARGS__))

 /**
 * \brief Get first logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogA("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_A (VLog::GetInstance().GetCustomLogA())

 /**
 * \brief Get second logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogB("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_B (VLog::GetInstance().GetCustomLogB())

 /**
 * \brief Get third logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogC("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_C (VLog::GetInstance().GetCustomLogC())

 /**
 * \brief Get fourth logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogD("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_D (VLog::GetInstance().GetCustomLogD())

 /**
 * \brief Get warning logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogW("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_W (VLog::GetInstance().GetWarningLog())

 /**
 * \brief Get error logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogR("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_R (VLog::GetInstance().GetErrorLog())

 /**
 * \brief Get info logger (up to 8 string-convertable parameters) with custom file path and line number
 * \param[in] first Custom file name
 * \param[in] second Custom line number
 * \param[in] other String-convertable params
 * \remarks VLogI("C:/Proj/file.cpp", 12, "a", 15, 3.0);
 */
#define V_LOG_I (VLog::GetInstance().GetInfoLog())

/*-----------------------------------------------------------------------------------------------------------------------------------*/
 /** \brief [INTERNAL] Used to determine macro to use, up to 8 arguments */
#define INTERNAL_get_check_macro_8(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
 /** \brief [INTERNAL] Wrapper for INTERNAL_get_check_macro_8 you need to use in VS */
#define INTERNAL_expand(x) x
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogA1(a) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogA2(a, b) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogA3(a, b, c) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogA4(a, b, c, d) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogA5(a, b, c, d, e) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogA6(a, b, c, d, e, f) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogA7(a, b, c, d, e, f, g) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogA8(a, b, c, d, e, f, g, h) V_LOG_A(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogB1(a) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogB2(a, b) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogB3(a, b, c) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogB4(a, b, c, d) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogB5(a, b, c, d, e) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogB6(a, b, c, d, e, f) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogB7(a, b, c, d, e, f, g) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogB8(a, b, c, d, e, f, g, h) V_LOG_B(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogC1(a) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogC2(a, b) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogC3(a, b, c) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogC4(a, b, c, d) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogC5(a, b, c, d, e) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogC6(a, b, c, d, e, f) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogC7(a, b, c, d, e, f, g) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogC8(a, b, c, d, e, f, g, h) V_LOG_C(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogD1(a) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogD2(a, b) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogD3(a, b, c) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogD4(a, b, c, d) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogD5(a, b, c, d, e) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogD6(a, b, c, d, e, f) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogD7(a, b, c, d, e, f, g) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogD8(a, b, c, d, e, f, g, h) V_LOG_D(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogW1(a) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogW2(a, b) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogW3(a, b, c) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogW4(a, b, c, d) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogW5(a, b, c, d, e) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogW6(a, b, c, d, e, f) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogW7(a, b, c, d, e, f, g) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogW8(a, b, c, d, e, f, g, h) V_LOG_W(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogR1(a) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogR2(a, b) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogR3(a, b, c) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogR4(a, b, c, d) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogR5(a, b, c, d, e) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogR6(a, b, c, d, e, f) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogR7(a, b, c, d, e, f, g) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogR8(a, b, c, d, e, f, g, h) V_LOG_R(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_LogI1(a) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a)
#define INTERNAL_LogI2(a, b) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b)
#define INTERNAL_LogI3(a, b, c) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c)
#define INTERNAL_LogI4(a, b, c, d) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d)
#define INTERNAL_LogI5(a, b, c, d, e) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e)
#define INTERNAL_LogI6(a, b, c, d, e, f) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f)
#define INTERNAL_LogI7(a, b, c, d, e, f, g) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g)
#define INTERNAL_LogI8(a, b, c, d, e, f, g, h) V_LOG_I(INTERNAL_FILE_, INTERNAL_LINE_, a, b, c, d, e, f, g, h)
/*-----------------------------------------------------------------------------------------------------------------------------------*/
#define INTERNAL_FILE_ VLog::GetRelativeFileName(__FILE__)
#define INTERNAL_LINE_ VLog::GetLine(__LINE__)

using namespace std;

class LoggerInit
{
public:
	fstream LogFile;
	LoggerInit() {
		CreateDirectory("../Logs", NULL);
		string prefix = "../Logs/";
		string activeLogFile = prefix;
		activeLogFile.append("Current.flog");
		string name;
		fstream tmp = fstream(activeLogFile, fstream::in);
		getline(tmp, name);
		if (name != "") {
			string tmpStr = prefix;
			tmpStr.append(name);
			name = tmpStr;
			fstream oldLog = fstream(name, fstream::out);
			while (getline(tmp, tmpStr)) oldLog << tmpStr << '\n';
		}
		tmp.close();
		LogFile = fstream(activeLogFile, fstream::out | fstream::trunc);
		LogFile << time(0) << ".flog\n";
	}
};

class Logger
{
private:
	string type;
	LoggerInit* TemporaryLogFile;

	template<class DT, class ...T>
	void LogHelper(DT arg, T... args) {
		TemporaryLogFile->LogFile << arg << "|";
		LogHelper(args...);
	}

	template<class DT>
	void LogHelper(DT arg) {
		TemporaryLogFile->LogFile << arg;
	}

public:
	Logger() {}
	Logger(string _type, LoggerInit* TmpLogFile) : type(_type), TemporaryLogFile(TmpLogFile) {}

	template<class ...T>
	void operator() (T... args) {
		TemporaryLogFile->LogFile << type;
		LogHelper(args...);
		TemporaryLogFile->LogFile << endl;
	}

	template<class T>
	Logger& operator << (T arg) {
		TemporaryLogFile->LogFile << type << arg << endl;
		return *this;
	}
};

class VLog
{
private:
	VLog() {
		LoggerInit* logFile = new LoggerInit();

		LogA = new Logger("LogA|", logFile);
		LogB = new Logger("LogB|", logFile);
		LogC = new Logger("LogC|", logFile);
		LogD = new Logger("LogD|", logFile);

		LogW = new Logger("Warning|", logFile);
		LogR = new Logger("Error|", logFile);
		LogI = new Logger("Info|", logFile);
	}

	Logger* LogA;
	Logger* LogB;
	Logger* LogC;
	Logger* LogD;

	Logger* LogW;
	Logger* LogR;
	Logger* LogI;

public:
	VLog(VLog const&) = delete;
	void operator=(VLog const&) = delete;

	static VLog& GetInstance() {
		static VLog instance;
		return instance;
	}

	static string GetLine(int LineNr) {
		return std::to_string(LineNr);
		//return ("Line: " + to_string(LineNr));
	}

	static string GetRelativeFileName(string FullPath) {
		return FullPath;

		size_t lastDelimiterPos = FullPath.find_last_of('\\');
		if (lastDelimiterPos == string::npos) {
			lastDelimiterPos = FullPath.find_last_of('/');
			if (lastDelimiterPos == string::npos)
				return FullPath;
		}

		size_t substringStartPos = FullPath.find_last_of('\\', lastDelimiterPos - 1);
		if (substringStartPos == string::npos) {
			substringStartPos = FullPath.find_last_of('/', lastDelimiterPos - 1);
			if (substringStartPos == string::npos)
				return FullPath;
		}

		size_t substringStartPosEarlier = FullPath.find_last_of('\\', substringStartPos - 1);
		if (substringStartPosEarlier == string::npos) {
			substringStartPosEarlier = FullPath.find_last_of('/', substringStartPos - 1);
			if (substringStartPosEarlier == string::npos)
				return FullPath;
		}

		return FullPath.substr(substringStartPosEarlier + 1, FullPath.length() - substringStartPosEarlier - 1);
	}

	Logger& GetCustomLogA() const { return *LogA; }
	Logger& GetCustomLogB() const { return *LogB; }
	Logger& GetCustomLogC() const { return *LogC; }
	Logger& GetCustomLogD() const { return *LogD; }

	Logger& GetWarningLog() const { return *LogW; }
	Logger& GetErrorLog() const { return *LogR; }
	Logger& GetInfoLog() const { return *LogI; }
};

#endif