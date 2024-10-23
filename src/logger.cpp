#include "../include/logger.hpp"

Logger::Logger() : Logger("log.txt")
{
    
}

Logger::Logger(const std::string filename)
{
    this->filename = filename;

    log_file.open(filename, std::ios::app);

    if (!log_file.is_open()) {
        std::cerr << "ERROR OPENING FILE" << std::endl;
    }

    cnt_new_files = 1;
    current_record_count = 0;
    max_records_per_file = 1000000;
}

Logger::~Logger()
{
    if (log_file.is_open()) {
        log_file.close();
    }

    std::cout << "LOGGER SERVER" << std::endl;
}

// Logs a message with a given log level
void Logger::log(log_level level, const std::string& ip, int port, const std::string& message)
{

    if (message.empty())
        return;

    time_t now = time(0);
    tm* timeinfo = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp),
            "%Y-%m-%d %H:%M:%S", timeinfo);

    // Create log entry
    log_file << "[" << timestamp << "] ";
    log_file << "[" << ip << ":" << port << "] ";
    log_file << levelToString(level) << ": ";
    log_file << message << std::endl;

    current_record_count++;

    if (current_record_count >= max_records_per_file) {
        log_file.close();

        std::string filename = std::to_string(cnt_new_files) + this->filename;
        log_file.open(filename, std::ios::app);
        cnt_new_files++;
        current_record_count = 0;
    }
}

std::string Logger::levelToString(log_level level)
{
    switch (level) {
    case DEBUG:
        return "DEBUG";
    case INFO:
        return "INFO";
    case ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}