#ifndef SMARTWASTE_HARDWARE_SIMULATEDCONVEYOR_HPP
#define SMARTWASTE_HARDWARE_SIMULATEDCONVEYOR_HPP

#include "../hal/IConveyor.hpp"
#include "HardwareLock.hpp"

namespace smartwaste::hardware {

/**
 * @brief Thread-safe simulated conveyor belt implementing IConveyor.
 * Models variable conveyor speed and stopping mechanics without physical motor drivers.
 */
class SimulatedConveyor : public smartwaste::hal::IConveyor {
public:
    explicit SimulatedConveyor(double maxSpeed = 5.0);
    ~SimulatedConveyor() override = default;

    // IConveyor interface implementation
    void setSpeed(double speed) override;
    double getSpeed() const override;
    void stop() override;

    // Simulation helper methods
    double getMaxSpeed() const;
    bool isRunning() const;

private:
    double m_currentSpeed{0.0};
    double m_maxSpeed{5.0};
    mutable Mutex m_mutex;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SIMULATEDCONVEYOR_HPP
