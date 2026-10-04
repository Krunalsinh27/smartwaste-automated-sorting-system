#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
#else
#include <mutex>
#endif

namespace smartwaste::logging {

struct Logger::LockImpl {
#if defined(_WIN32) || defined(_WIN64)
    CRITICAL_SECTION cs;
    LockImpl() { InitializeCriticalSection(&cs); }
    ~LockImpl() { DeleteCriticalSection(&cs); }
    void lock() { EnterCriticalSection(&cs); }
    void unlock() { LeaveCriticalSection(&cs); }
#else
    std::mutex mtx;
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
#endif
};

std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LOG_DEBUG_LEVEL:    return "DEBUG";
        case LogLevel::LOG_INFO_LEVEL:     return "INFO";
        case LogLevel::LOG_WARN_LEVEL:     return "WARNING";
        case LogLevel::LOG_ERROR_LEVEL:    return "ERROR";
        case LogLevel::LOG_CRITICAL_LEVEL: return "CRITICAL";
        default:                           return "UNKNOWN";
    }
}

LogLevel stringToLogLevel(const std::string& str) {
    if (str == "DEBUG" || str == "LOG_DEBUG_LEVEL")       return LogLevel::LOG_DEBUG_LEVEL;
    if (str == "INFO" || str == "LOG_INFO_LEVEL")         return LogLevel::LOG_INFO_LEVEL;
    if (str == "WARN" || str == "WARNING" || str == "LOG_WARN_LEVEL") return LogLevel::LOG_WARN_LEVEL;
    if (str == "ERROR" || str == "LOG_ERROR_LEVEL")       return LogLevel::LOG_ERROR_LEVEL;
    if (str == "CRITICAL" || str == "LOG_CRITICAL_LEVEL") return LogLevel::LOG_CRITICAL_LEVEL;
    return LogLevel::LOG_INFO_LEVEL;
}

Logger::Logger() : m_lockImpl(std::make_unique<LockImpl>()) {}

Logger::~Logger() {
    lock();
    if (m_fileStream.is_open()) {
        m_fileStream.flush();
        m_fileStream.close();
    }
    unlock();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::lock() {
    if (m_lockImpl) {
        m_lockImpl->lock();
    }
}

void Logger::unlock() {
    if (m_lockImpl) {
        m_lockImpl->unlock();
    }
}

void Logger::setLogLevel(LogLevel level) {
    lock();
    m_minLevel = level;
    unlock();
}

bool Logger::setLogFile(const std::string& filepath) {
    lock();
    if (m_fileStream.is_open()) {
        m_fileStream.close();
    }
    m_fileStream.open(filepath, std::ios::out | std::ios::app);
    bool isOk = m_fileStream.is_open();
    unlock();
    return isOk;
}

void Logger::log(LogLevel level, const std::string& tag, const std::string& message) {
    lock();

    if (static_cast<int>(level) < static_cast<int>(m_minLevel)) {
        unlock();
        return;
    }

    std::string timestamp = getCurrentTimestamp();
    std::string levelStr = logLevelToString(level);

    std::ostringstream formatted;
    formatted << "[" << timestamp << "] [" << levelStr << "]";
    if (!tag.empty()) {
        formatted << " [" << tag << "]";
    }
    formatted << " " << message;

    std::string output = formatted.str();

    // Print to console sink (cerr for error/critical, cout for others)
    if (level == LogLevel::LOG_ERROR_LEVEL || level == LogLevel::LOG_CRITICAL_LEVEL) {
        std::cerr << output << std::endl;
    } else {
        std::cout << output << std::endl;
    }

    // Print to file sink if active
    if (m_fileStream.is_open()) {
        m_fileStream << output << "\n";
        m_fileStream.flush();
    }

    unlock();
}

void Logger::flush() {
    lock();
    std::cout.flush();
    std::cerr.flush();
    if (m_fileStream.is_open()) {
        m_fileStream.flush();
    }
    unlock();
}

std::string Logger::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm* tmPtr = std::localtime(&timeT);

    std::ostringstream oss;
    if (tmPtr) {
        oss << std::put_time(tmPtr, "%Y-%m-%d %H:%M:%S");
    } else {
        oss << "1970-01-01 00:00:00";
    }
    return oss.str();
}

} // namespace smartwaste::logging
