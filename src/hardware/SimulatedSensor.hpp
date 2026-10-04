#ifndef SMARTWASTE_HARDWARE_SIMULATEDSENSOR_HPP
#define SMARTWASTE_HARDWARE_SIMULATEDSENSOR_HPP

#include "../hal/ISensor.hpp"
#include "HardwareLock.hpp"

namespace smartwaste::hardware {

/**
 * @brief Thread-safe simulated detection sensor implementing ISensor.
 * Allows deterministic signal setting and triggering for testing.
 */
class SimulatedSensor : public smartwaste::hal::ISensor {
public:
    explicit SimulatedSensor(double threshold = 0.5);
    ~SimulatedSensor() override = default;

    // ISensor interface implementation
    double readSignal() const override;
    bool isTriggered() const override;

    // Simulation helper methods
    void setSignal(double signal);
    void setTriggered(bool triggered);
    void setThreshold(double threshold);
    double getThreshold() const;

private:
    double m_signal{0.0};
    double m_threshold{0.5};
    bool m_forceTriggered{false};
    bool m_useForcedTrigger{false};
    mutable Mutex m_mutex;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SIMULATEDSENSOR_HPP
