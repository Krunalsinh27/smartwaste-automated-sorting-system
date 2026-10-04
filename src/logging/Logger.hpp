#ifndef SMARTWASTE_LOGGING_LOGGER_HPP
#define SMARTWASTE_LOGGING_LOGGER_HPP

#include <string>
#include <fstream>
#include <memory>

namespace smartwaste::logging {

enum class LogLevel {
    LOG_DEBUG_LEVEL,
    LOG_INFO_LEVEL,
    LOG_WARN_LEVEL,
    LOG_ERROR_LEVEL,
    LOG_CRITICAL_LEVEL
};

std::string logLevelToString(LogLevel level);
LogLevel stringToLogLevel(const std::string& str);

/**
 * @brief Thread-safe logger supporting console and file output sinks.
 */
class Logger {
public:
    static Logger& getInstance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Sets the minimum log level to output.
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief Enables writing log output to a file.
     * @param filepath Target log file path.
     * @return true if opened successfully, false otherwise.
     */
    bool setLogFile(const std::string& filepath);

    /**
     * @brief Logs a message with specified level and tag.
     */
    void log(LogLevel level, const std::string& tag, const std::string& message);

    void debug(const std::string& tag, const std::string& message)    { log(LogLevel::LOG_DEBUG_LEVEL, tag, message); }
    void info(const std::string& tag, const std::string& message)     { log(LogLevel::LOG_INFO_LEVEL, tag, message); }
    void warn(const std::string& tag, const std::string& message)     { log(LogLevel::LOG_WARN_LEVEL, tag, message); }
    void error(const std::string& tag, const std::string& message)    { log(LogLevel::LOG_ERROR_LEVEL, tag, message); }
    void critical(const std::string& tag, const std::string& message) { log(LogLevel::LOG_CRITICAL_LEVEL, tag, message); }

    /**
     * @brief Flushes all pending log streams.
     */
    void flush();

    ~Logger();

private:
    Logger();

    std::string getCurrentTimestamp() const;
    void lock();
    void unlock();

    LogLevel m_minLevel{LogLevel::LOG_INFO_LEVEL};
    std::ofstream m_fileStream;

    struct LockImpl;
    std::unique_ptr<LockImpl> m_lockImpl;
};

} // namespace smartwaste::logging

// Convenient logging macros
#define LOG_DEBUG(tag, msg)    ::smartwaste::logging::Logger::getInstance().debug(tag, msg)
#define LOG_INFO(tag, msg)     ::smartwaste::logging::Logger::getInstance().info(tag, msg)
#define LOG_WARN(tag, msg)     ::smartwaste::logging::Logger::getInstance().warn(tag, msg)
#define LOG_ERROR(tag, msg)    ::smartwaste::logging::Logger::getInstance().error(tag, msg)
#define LOG_CRITICAL(tag, msg) ::smartwaste::logging::Logger::getInstance().critical(tag, msg)

#endif // SMARTWASTE_LOGGING_LOGGER_HPP
