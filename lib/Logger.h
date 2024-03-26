#pragma once

#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <source_location>

#define _LOGFUNC_ LOGGER(LOGGER::INFO).LOGFUNC()

class LOGGER {
public:
    enum LOG_LEVEL {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERR = 3,
    };

    LOGGER(LOG_LEVEL level)
        : _outfile(_logger_file_path, std::ios_base::app), _log_level(level) {}

    ~LOGGER() {
        _outfile << '\n';
        _outfile.close();
    }

    void LOGFUNC(const std::source_location& location = std::source_location::current()) {
        if (_log_level >= _program_log_level) {
            _outfile << "[" << get_current_time() << "] ["
                << convert_log_level(_log_level) << "]: " << location.function_name();
            std::cout << "[" << get_current_time() << "] ["
                << convert_log_level(_log_level) << "]: " << location.function_name()
                << std::endl;
        }
    }

    template <class T>
    LOGGER& operator<<(const T& thing) {
        if (_log_level >= _program_log_level) {
            _outfile << "[" << get_current_time() << "] ["
                << convert_log_level(_log_level) <<  "] ["
                << std::source_location::current().function_name()
                << "]: " << thing;
            std::cout << "[" << get_current_time() << "] ["
                << convert_log_level(_log_level) << "] ["
                << std::source_location::current().function_name()
                << "]: " << thing
                << std::endl;
        }
        return *this;
    }

private:
    constexpr std::string_view convert_log_level(LOG_LEVEL level) {
        switch (level) {
        case LOG_LEVEL::DEBUG:
            return "DEBUG";
        case LOG_LEVEL::INFO:
            return "INFO";
        case LOG_LEVEL::ERR:
            return "ERROR";
        case LOG_LEVEL::WARNING:
            return "WARNING";
        default:
            exit(EXIT_FAILURE);
        }
    }

    std::string get_current_time() {
        auto t{ std::chrono::system_clock::now() };
        auto t_time_t{ std::chrono::system_clock::to_time_t(t) };
        std::string time(std::ctime(&t_time_t));
        size_t end = time.find_last_of("\n");
        time = time.substr(0, end);
        return time;
    }

    std::string _logger_file_path{ "./Logger.txt" };
    std::ofstream _outfile;
    LOG_LEVEL _log_level;
#ifdef NDEBUG
    LOG_LEVEL _program_log_level{ LOGGER::INFO };
#else
    LOG_LEVEL _program_log_level{ LOGGER::DEBUG };
#endif
};