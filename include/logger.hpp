#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

enum log_level {DEBUG, INFO, ERROR};

class Logger {
private:
    std::ofstream log_file;
    
    std::string filename;

    int cnt_new_files;
    int current_record_count;
    int max_records_per_file;

    // Converts log level to a string for output
    std::string levelToString(log_level level);

public:
    Logger();
    Logger(const std::string filename);

    ~Logger();

    // Logs a message with a given log level
    void log(log_level level, const std::string& ip, int port, const std::string& message);
};
