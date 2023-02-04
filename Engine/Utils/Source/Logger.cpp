#include "Logger.h"
#include <ctime>
#include <Windows.h>
#include <fstream>

using namespace std;
LoggerInit::LoggerInit() {
	CreateDirectory("../Logs", nullptr);
	const string prefix = "../Logs/";
	string activeLogFile = prefix;
	activeLogFile.append("Current.flog");
	string name;
	fstream tmp = fstream(activeLogFile, fstream::in);
	getline(tmp, name);
	if (!name.empty()) {
		string tmpStr = prefix + "old_";
		tmpStr.append(name);
		name = tmpStr;
		fstream oldLog = fstream(name, fstream::out);
		while (getline(tmp, tmpStr)) oldLog << tmpStr << '\n';
	}
	tmp.close();
	logFile = fstream(activeLogFile, fstream::out | fstream::trunc);
	logFile << time(nullptr) << ".flog\n";
}

Logger::Logger(): temporaryLogFile_(nullptr) {

}

Logger::Logger(std::string type, LoggerInit* tmpLogFile): temporaryLogFile_(tmpLogFile), type_(type) {
	
}

VLog& VLog::getInstance() {
	static VLog instance;
	return instance;
}

std::string VLog::getLine(int lineNr) {
	return std::to_string(lineNr);
}

std::string VLog::getRelativeFileName(std::string fullPath) {
	return fullPath;
}

VLog::VLog() {
	LoggerInit* logFile = new LoggerInit();

	logA_ = new Logger("LogA|", logFile);
	logB_ = new Logger("LogB|", logFile);
	logC_ = new Logger("LogC|", logFile);
	logD_ = new Logger("LogD|", logFile);

	logW_ = new Logger("Warning|", logFile);
	logR_ = new Logger("Error|", logFile);
	logI_ = new Logger("Info|", logFile);

	logConsole_ = new Logger("Console|", logFile);
}
