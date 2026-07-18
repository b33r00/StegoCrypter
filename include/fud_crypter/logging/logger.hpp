#pragma once

#include <string>
#include <memory>
#include <ostream>
#include <iostream>

namespace fud_crypter {

    enum class LogLevel {
        DEBUG,
        INFO,
        SUCCESS,
        WARNING,
        ERROR,
        FATAL
    };

    class Logger {
    public:
        Logger(LogLevel min_level = LogLevel::INFO,
               std::ostream& output = std::cerr)
        : min_level_(min_level), output_(output) {}

        // set_output törölve (ostream nem másolható)
        // Ha kell, használj pointert, de jelenleg nem szükséges

        void log(LogLevel level, const std::string& message) {
            if (level < min_level_) return;
            output_ << prefix_for(level) << message << std::endl;
        }

        void debug(const std::string& msg)   { log(LogLevel::DEBUG, msg); }
        void info(const std::string& msg)    { log(LogLevel::INFO, msg); }
        void success(const std::string& msg) { log(LogLevel::SUCCESS, msg); }
        void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
        void error(const std::string& msg)   { log(LogLevel::ERROR, msg); }
        void fatal(const std::string& msg)   { log(LogLevel::FATAL, msg); }

    private:
        static const char* prefix_for(LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG:   return "[*] ";
                case LogLevel::INFO:    return "[i] ";
                case LogLevel::SUCCESS: return "[+] ";
                case LogLevel::WARNING: return "[!] ";
                case LogLevel::ERROR:   return "[-] ";
                case LogLevel::FATAL:   return "[X] ";
                default:                return "[?] ";
            }
        }

        LogLevel min_level_;
        std::ostream& output_;
    };

    using LoggerPtr = std::shared_ptr<Logger>;

    inline LoggerPtr create_logger(LogLevel level = LogLevel::INFO,
                                   std::ostream& output = std::cerr) {
        return std::make_shared<Logger>(level, output);
                                   }

} // namespace fud_crypter
