#include "SQLiteDatabase.hpp"
extern "C" {
#include "sqlite3.h"
}

#include "../logging/Logger.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace smartwaste::database {

using smartwaste::hardware::ScopedLock;

static bool ensureDirectoryExists(const std::string& filePath) {
    std::size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return true;
    }
    std::string dir = filePath.substr(0, lastSlash);
    if (dir.empty()) {
        return true;
    }
#if defined(_WIN32) || defined(_WIN64)
    _mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0755);
#endif
    return true;
}

SQLiteDatabase::~SQLiteDatabase() {
    close();
}

SQLiteDatabase::SQLiteDatabase(SQLiteDatabase&& other) noexcept {
    ScopedLock lock(other.m_mutex);
    m_db = other.m_db;
    m_dbPath = std::move(other.m_dbPath);
    m_lastError = std::move(other.m_lastError);
    m_inTransaction = other.m_inTransaction;

    other.m_db = nullptr;
    other.m_inTransaction = false;
}

SQLiteDatabase& SQLiteDatabase::operator=(SQLiteDatabase&& other) noexcept {
    if (this != &other) {
        close();
        ScopedLock lockThis(m_mutex);
        ScopedLock lockOther(other.m_mutex);

        m_db = other.m_db;
        m_dbPath = std::move(other.m_dbPath);
        m_lastError = std::move(other.m_lastError);
        m_inTransaction = other.m_inTransaction;

        other.m_db = nullptr;
        other.m_inTransaction = false;
    }
    return *this;
}

bool SQLiteDatabase::open(const std::string& dbPath) {
    ScopedLock lock(m_mutex);

    if (m_db != nullptr) {
        if (m_dbPath == dbPath) {
            return true;
        }
        close();
    }

    m_dbPath = dbPath;
    m_lastError.clear();

    // Ensure parent directory exists
    if (!ensureDirectoryExists(dbPath)) {
        setLastError("Failed to create database directory for path: " + dbPath);
        LOG_ERROR("SQLiteDatabase", m_lastError);
        return false;
    }

    int rc = sqlite3_open_v2(dbPath.c_str(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        setLastError(sqlite3_errstr(rc));
        LOG_ERROR("SQLiteDatabase", "Failed to open SQLite database at " + dbPath + ": " + m_lastError);
        if (m_db) {
            sqlite3_close(m_db);
            m_db = nullptr;
        }
        return false;
    }

    // Enable foreign keys
    char* errMsgs = nullptr;
    rc = sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        LOG_WARN("SQLiteDatabase", "Failed to enable foreign keys: " + m_lastError);

    }

    LOG_INFO("SQLiteDatabase", "Successfully opened SQLite database at: " + dbPath);

    // Idempotently initialize database schema
    return initializeSchema();
}

void SQLiteDatabase::close() {
    ScopedLock lock(m_mutex);
    if (m_db != nullptr) {
        if (m_inTransaction) {
            sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, nullptr);
            m_inTransaction = false;
        }
        sqlite3_close(m_db);
        m_db = nullptr;
        LOG_INFO("SQLiteDatabase", "Closed SQLite database connection: " + m_dbPath);
    }
}

bool SQLiteDatabase::isOpen() const noexcept {
    ScopedLock lock(m_mutex);
    return m_db != nullptr;
}

bool SQLiteDatabase::execute(const std::string& sql) {
    ScopedLock lock(m_mutex);
    if (!m_db) {
        setLastError("Database is not open.");
        return false;
    }

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        } else {
            setLastError(sqlite3_errmsg(m_db));
        }
        LOG_ERROR("SQLiteDatabase", "SQL execution failed: " + m_lastError + " | Query: " + sql);
        return false;
    }

    return true;
}

bool SQLiteDatabase::beginTransaction() {
    ScopedLock lock(m_mutex);
    if (!m_db) {
        setLastError("Database is not open.");
        return false;
    }
    if (m_inTransaction) {
        setLastError("Transaction already active.");
        return false;
    }

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        return false;
    }

    m_inTransaction = true;
    return true;
}

bool SQLiteDatabase::commitTransaction() {
    ScopedLock lock(m_mutex);
    if (!m_db) {
        setLastError("Database is not open.");
        return false;
    }
    if (!m_inTransaction) {
        setLastError("No active transaction to commit.");
        return false;
    }

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, "COMMIT;", nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        return false;
    }

    m_inTransaction = false;
    return true;
}

bool SQLiteDatabase::rollbackTransaction() {
    ScopedLock lock(m_mutex);
    if (!m_db) {
        setLastError("Database is not open.");
        return false;
    }
    if (!m_inTransaction) {
        setLastError("No active transaction to rollback.");
        return false;
    }

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, "ROLLBACK;", nullptr, nullptr, &errMsgs);
    m_inTransaction = false;

    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        return false;
    }

    return true;
}

int SQLiteDatabase::getSchemaVersion() const {
    ScopedLock lock(m_mutex);
    if (!m_db) return 0;

    const char* sql = "SELECT version FROM schema_version LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int version = 0;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            version = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return version;
}

bool SQLiteDatabase::setSchemaVersion(int version) {
    ScopedLock lock(m_mutex);
    if (!m_db) return false;

    auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::string sql = "DELETE FROM schema_version; INSERT INTO schema_version (version, updated_at) VALUES (" +
                      std::to_string(version) + ", " + std::to_string(nowMs) + ");";

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        return false;
    }
    return true;
}

bool SQLiteDatabase::initializeSchema() {
    const char* createSchemaSql = R"SQL(
        CREATE TABLE IF NOT EXISTS schema_version (
            version INTEGER PRIMARY KEY,
            updated_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS machine_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            event_id TEXT UNIQUE NOT NULL,
            timestamp_ms INTEGER NOT NULL,
            event_type TEXT NOT NULL,
            severity TEXT NOT NULL,
            message TEXT NOT NULL,
            item_id TEXT DEFAULT '',
            waste_type TEXT DEFAULT 'UNKNOWN',
            gate_id INTEGER DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS waste_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            item_id TEXT UNIQUE NOT NULL,
            timestamp_ms INTEGER NOT NULL,
            waste_type TEXT NOT NULL,
            confidence REAL NOT NULL,
            gate_id INTEGER DEFAULT 0,
            bbox_x INTEGER DEFAULT 0,
            bbox_y INTEGER DEFAULT 0,
            bbox_width INTEGER DEFAULT 0,
            bbox_height INTEGER DEFAULT 0,
            sorting_status TEXT DEFAULT 'UNKNOWN',
            created_at INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS machine_operations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            operation TEXT NOT NULL,
            previous_state TEXT NOT NULL,
            new_state TEXT NOT NULL,
            timestamp_ms INTEGER NOT NULL,
            message TEXT DEFAULT ''
        );

        CREATE TABLE IF NOT EXISTS simulation_runs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            started_at INTEGER NOT NULL,
            completed_at INTEGER NOT NULL,
            total_items INTEGER DEFAULT 0,
            successful_items INTEGER DEFAULT 0,
            rejected_items INTEGER DEFAULT 0,
            unavailable_gate_items INTEGER DEFAULT 0,
            failed_items INTEGER DEFAULT 0,
            total_gate_actuations INTEGER DEFAULT 0,
            health_checks_performed INTEGER DEFAULT 0,
            healthy_checks INTEGER DEFAULT 0,
            warning_checks INTEGER DEFAULT 0,
            critical_checks INTEGER DEFAULT 0
        );
    )SQL";

    char* errMsgs = nullptr;
    int rc = sqlite3_exec(m_db, createSchemaSql, nullptr, nullptr, &errMsgs);
    if (rc != SQLITE_OK) {
        if (errMsgs) {
            setLastError(errMsgs);
            sqlite3_free(errMsgs);
        }
        LOG_ERROR("SQLiteDatabase", "Failed to initialize foundational schema: " + m_lastError);
        return false;
    }

    // Set schema version to 1 if not set
    const char* checkVerSql = "SELECT COUNT(*) FROM schema_version;";
    sqlite3_stmt* stmt = nullptr;
    int count = 0;
    if (sqlite3_prepare_v2(m_db, checkVerSql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string initVerSql = "INSERT INTO schema_version (version, updated_at) VALUES (1, " +
                                 std::to_string(nowMs) + ");";
        sqlite3_exec(m_db, initVerSql.c_str(), nullptr, nullptr, nullptr);
    }

    LOG_INFO("SQLiteDatabase", "Foundational SQLite schema initialized successfully (Version 1).");
    return true;
}

bool SQLiteDatabase::tableExists(const std::string& tableName) const {
    ScopedLock lock(m_mutex);
    if (!m_db) return false;

    std::string sql = "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* stmt = nullptr;
    bool exists = false;

    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = (sqlite3_column_int(stmt, 0) > 0);
        }
        sqlite3_finalize(stmt);
    }
    return exists;
}

std::string SQLiteDatabase::getLastError() const {
    ScopedLock lock(m_mutex);
    return m_lastError;
}

std::string SQLiteDatabase::getDatabasePath() const {
    ScopedLock lock(m_mutex);
    return m_dbPath;
}

void SQLiteDatabase::setLastError(const std::string& err) {
    m_lastError = err;
}

} // namespace smartwaste::database
