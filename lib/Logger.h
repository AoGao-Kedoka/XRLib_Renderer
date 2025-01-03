/*
* Author: AoGao-Kedoka
*/

#pragma once
#include <chrono>
#include <fstream>
#include <iostream>
#include <source_location>
#include <sstream>
#include <mutex>

#define _LOGFUNC_ LOGGER(LOGGER::INFO).LOGFUNC()

class LOGGER {
   public:
    enum LOG_LEVEL {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERR = 3,
    };

    LOGGER(LOG_LEVEL level, const std::source_location& location =
                                std::source_location::current())
        : _log_level(level), _location(location) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_log_level >= _program_log_level) {
            _outfile.open(_logger_file_path, std::ios_base::app);
            _outfile << "[" << get_current_time() << "] ["
                     << convert_log_level(_log_level) << "] ["
                     << _location.function_name() << "]: ";
            std::cout << log_color(level, true) << "[" << get_current_time()
                      << "] [" << convert_log_level(_log_level) << "] ["
                      << _location.function_name()
                      << "]: " << log_color(level, false);
        }
    }

    ~LOGGER() {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_log_level >= _program_log_level) {
            std::cout << '\n';
            _outfile << '\n';
            _outfile.close();
        }
    }

    void LOGFUNC(const std::source_location& location =
                     std::source_location::current()) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_log_level >= _program_log_level) {
            _outfile << "";
            std::cout << "";
        }
    }

    template <class T>
    LOGGER& operator<<(const T& thing) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_log_level >= _program_log_level) {
            _outfile << thing;
            std::cout << thing;
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

    constexpr std::string_view log_color(LOG_LEVEL level, bool start) {
        if (!start)
            return "\033[0m";

        switch (level) {
            case LOG_LEVEL::DEBUG:
                return "\033[1;36m";
            case LOG_LEVEL::INFO:
                return "";
            case LOG_LEVEL::ERR:
                return "\033[1;31m";
            case LOG_LEVEL::WARNING:
                return "\033[1;33m";
            default:
                exit(EXIT_FAILURE);
        }
    }

    std::string get_current_time() {
        auto t{std::chrono::system_clock::now()};
        auto t_time_t{std::chrono::system_clock::to_time_t(t)};
        std::string time(std::ctime(&t_time_t));
        size_t end = time.find_last_of("\n");
        time = time.substr(0, end);
        return time;
    }

    static inline std::mutex _mutex;
    static inline std::string _logger_file_path{"./XRLIB_LOG.txt"};
    static inline std::ofstream _outfile;
    LOG_LEVEL _log_level;
    std::source_location _location;
#ifdef NDEBUG
    LOG_LEVEL _program_log_level{LOGGER::INFO};
#else
    LOG_LEVEL _program_log_level{LOGGER::DEBUG};
#endif
};
