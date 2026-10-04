#ifndef SMARTWASTE_HARDWARE_HARDWARELOCK_HPP
#define SMARTWASTE_HARDWARE_HARDWARELOCK_HPP

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

namespace smartwaste::hardware {

class Mutex {
public:
    Mutex() {
#if defined(_WIN32) || defined(_WIN64)
        InitializeCriticalSection(&m_cs);
#endif
    }
    ~Mutex() {
#if defined(_WIN32) || defined(_WIN64)
        DeleteCriticalSection(&m_cs);
#endif
    }
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    void lock() {
#if defined(_WIN32) || defined(_WIN64)
        EnterCriticalSection(&m_cs);
#else
        m_mtx.lock();
#endif
    }

    void unlock() {
#if defined(_WIN32) || defined(_WIN64)
        LeaveCriticalSection(&m_cs);
#else
        m_mtx.unlock();
#endif
    }

private:
#if defined(_WIN32) || defined(_WIN64)
    CRITICAL_SECTION m_cs;
#else
    std::mutex m_mtx;
#endif
};

class ScopedLock {
public:
    explicit ScopedLock(Mutex& mtx) : m_mtx(mtx) { m_mtx.lock(); }
    ~ScopedLock() { m_mtx.unlock(); }
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;

private:
    Mutex& m_mtx;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_HARDWARELOCK_HPP
