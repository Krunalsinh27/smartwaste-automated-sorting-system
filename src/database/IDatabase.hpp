#ifndef SMARTWASTE_DATABASE_IDATABASE_HPP
#define SMARTWASTE_DATABASE_IDATABASE_HPP

#include <string>

namespace smartwaste::database {

/**
 * @brief Pure virtual interface defining database abstraction operations for SmartWaste storage layer.
 */
class IDatabase {
public:
    virtual ~IDatabase() = default;

    /**
     * @brief Opens or creates a database at the specified file path.
     * @param dbPath Absolute or relative filesystem path to database file.
     * @return true if opened and initialized successfully, false otherwise.
     */
    virtual bool open(const std::string& dbPath) = 0;

    /**
     * @brief Safely closes the database connection and releases resources.
     */
    virtual void close() = 0;

    /**
     * @brief Checks if database connection is currently active.
     */
    virtual bool isOpen() const noexcept = 0;

    /**
     * @brief Executes a raw SQL statement (DDL/DML).
     * @param sql Valid SQL query string.
     * @return true if execution succeeded, false otherwise.
     */
    virtual bool execute(const std::string& sql) = 0;

    /**
     * @brief Begins an explicit transaction block.
     */
    virtual bool beginTransaction() = 0;

    /**
     * @brief Commits current transaction block.
     */
    virtual bool commitTransaction() = 0;

    /**
     * @brief Rolls back current transaction block.
     */
    virtual bool rollbackTransaction() = 0;

    /**
     * @brief Retrieves current schema version number.
     * @return Version integer (0 if uninitialized or error).
     */
    virtual int getSchemaVersion() const = 0;

    /**
     * @brief Updates stored schema version number.
     */
    virtual bool setSchemaVersion(int version) = 0;

    /**
     * @brief Returns human-readable message of last encountered database error.
     */
    virtual std::string getLastError() const = 0;

    /**
     * @brief Returns active database filesystem path.
     */
    virtual std::string getDatabasePath() const = 0;
};

} // namespace smartwaste::database

#endif // SMARTWASTE_DATABASE_IDATABASE_HPP
