#ifndef SMARTWASTE_HARDWARE_HARDWAREMANAGER_HPP
#define SMARTWASTE_HARDWARE_HARDWAREMANAGER_HPP

#include "../hal/IConveyor.hpp"
#include "../hal/IMotor.hpp"
#include "../hal/ISensor.hpp"
#include "../hal/IGate.hpp"
#include "HardwareLock.hpp"

#include <vector>
#include <cstddef>

namespace smartwaste::hardware {

/**
 * @brief Thread-safe hardware manager coordinating multiple gates, sensors,
 * main conveyor, and drive motor strictly via HAL pure virtual interface abstractions.
 */
class HardwareManager {
public:
    HardwareManager(
        smartwaste::hal::IConveyor& conveyor,
        smartwaste::hal::IMotor& motor
    );

    ~HardwareManager() = default;

    HardwareManager(const HardwareManager&) = delete;
    HardwareManager& operator=(const HardwareManager&) = delete;

    // Registration
    void addGate(smartwaste::hal::IGate& gate);
    void addSensor(smartwaste::hal::ISensor& sensor);

    // Hardware Coordination
    bool actuateGate(std::size_t gateIndex);
    bool isGateJammed(std::size_t gateIndex) const;
    bool hasAnyGateJam() const;

    // Safety shutdown
    void stopAll();

    // Accessors
    std::size_t getGateCount() const;
    std::size_t getSensorCount() const;
    
    smartwaste::hal::IConveyor& getConveyor() const noexcept { return m_conveyor; }
    smartwaste::hal::IMotor& getMotor() const noexcept { return m_motor; }

    smartwaste::hal::IGate* getGate(std::size_t gateIndex) const;
    smartwaste::hal::ISensor* getSensor(std::size_t sensorIndex) const;

private:
    smartwaste::hal::IConveyor& m_conveyor;
    smartwaste::hal::IMotor& m_motor;

    std::vector<smartwaste::hal::IGate*> m_gates;
    std::vector<smartwaste::hal::ISensor*> m_sensors;

    mutable Mutex m_mutex;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_HARDWAREMANAGER_HPP
