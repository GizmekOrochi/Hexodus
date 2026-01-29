#include "../../include/util/Logger.hpp"

static std::ofstream logFile;

void Logger::log(const std::string& msg) {
    if (!logFile.is_open()) logFile.open("log.txt", std::ios::out | std::ios::trunc);
    logFile << msg << '\n';
    logFile.flush();
}
