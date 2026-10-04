#ifndef SMARTWASTE_DATABASE_SQLITEDATABASE_HPP
#define SMARTWASTE_DATABASE_SQLITEDATABASE_HPP

#include "IDatabase.hpp"
#include "../hardware/HardwareLock.hpp"
#include <string>

extern "C" {
struct sqlite3;
}


namespace smartwaste::database {

/**
 * @brief Concrete SQLite implementation of IDatabase supporting RAII connection management,
 * idempotent schema creation, transaction scope controls, and error logging.
 */
class SQLiteDatabase : public IDatabase {
public:
    SQLiteDatabase() = default;
    ~SQLiteDatabase() override;

    // Disable copy semantics to prevent duplicate handle manipulation
    SQLiteDatabase(const SQLiteDatabase&) = delete;
    SQLiteDatabase& operator=(const SQLiteDatabase&) = delete;

    // Enable move semantics
    SQLiteDatabase(SQLiteDatabase&& other) noexcept;
    SQLiteDatabase& operator=(SQLiteDatabase&& other) noexcept;

    bool open(const std::string& dbPath) override;
    void close() override;
    bool isOpen() const noexcept override;
    bool execute(const std::string& sql) override;

    bool beginTransaction() override;
    bool commitTransaction() override;
    bool rollbackTransaction() override;

    int getSchemaVersion() const override;
    bool setSchemaVersion(int version) override;

    std::string getLastError() const override;
    std::string getDatabasePath() const override;

    /**
     * @brief Idempotently initializes foundational database schema and tables.
     * @return true if schema is valid and ready, false on failure.
     */
    bool initializeSchema();

    /**
     * @brief Helper to check if a table exists in the database.
     */
    bool tableExists(const std::string& tableName) const;

private:
    void setLastError(const std::string& err);

    sqlite3* m_db{nullptr};
    std::string m_dbPath;
    mutable std::string m_lastError;
    mutable smartwaste::hardware::Mutex m_mutex;
    bool m_inTransaction{false};
};


/**
 * @brief RAII helper for managing SQLite transactions automatically.
 */
class TransactionScope {
public:
    explicit TransactionScope(IDatabase& db)
        : m_db(db) {
        m_active = m_db.beginTransaction();
    }

    ~TransactionScope() {
        if (m_active && !m_committed) {
            m_db.rollbackTransaction();
        }
    }

    bool commit() {
        if (m_active && !m_committed) {
            m_committed = m_db.commitTransaction();
            return m_committed;
        }
        return false;
    }

    bool isActive() const noexcept { return m_active; }

private:
    IDatabase& m_db;
    bool m_active{false};
    bool m_committed{false};
};

} // namespace smartwaste::database

#endif // SMARTWASTE_DATABASE_SQLITEDATABASE_HPP
