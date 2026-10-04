#ifndef SMARTWASTE_HARDWARE_SIMULATEDMOTOR_HPP
#define SMARTWASTE_HARDWARE_SIMULATEDMOTOR_HPP

#include "../hal/IMotor.hpp"
#include "HardwareLock.hpp"

namespace smartwaste::hardware {

/**
 * @brief Thread-safe simulated drive motor implementing IMotor.
 * Models rotational speed (RPM), start/stop states, and simulated fault injection.
 */
class SimulatedMotor : public smartwaste::hal::IMotor {
public:
    explicit SimulatedMotor(double targetRPM = 1500.0);
    ~SimulatedMotor() override = default;

    // IMotor interface implementation
    void start() override;
    void stop() override;
    double getRPM() const override;

    // Simulation & Fault Injection methods
    bool isRunning() const;
    double getTargetRPM() const;
    void setTargetRPM(double targetRPM);
    
    void setAbnormalState(bool abnormal);
    bool isAbnormalState() const;
    double getTemperature() const;

private:
    bool m_isRunning{false};
    double m_currentRPM{0.0};
    double m_targetRPM{1500.0};
    bool m_isAbnormal{false};
    double m_temperatureC{25.0};
    mutable Mutex m_mutex;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SIMULATEDMOTOR_HPP
