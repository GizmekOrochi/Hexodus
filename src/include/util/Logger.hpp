#ifndef LOGGER_HPP
#define LOGGER_HPP

#pragma once
#include <fstream>
#include <string>

class Logger {
public:
    static void log(const std::string& msg);
};


#endif // LOGGER_HPP
