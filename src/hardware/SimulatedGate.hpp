#ifndef SMARTWASTE_HARDWARE_SIMULATEDGATE_HPP
#define SMARTWASTE_HARDWARE_SIMULATEDGATE_HPP

#include "../hal/IGate.hpp"
#include "HardwareLock.hpp"

namespace smartwaste::hardware {

/**
 * @brief Thread-safe simulated pneumatic diverter gate implementing IGate.
 * Models gate actuation, actuation count metrics, and simulated mechanical jam fault injection.
 */
class SimulatedGate : public smartwaste::hal::IGate {
public:
    explicit SimulatedGate(int gateId = 1);
    ~SimulatedGate() override = default;

    // IGate interface implementation
    void actuate() override;
    bool isJamDetected() const override;

    // Simulation & Fault Injection helper methods
    int getGateId() const;
    int getActuationCount() const;
    bool wasActuated() const;
    void resetActuatedFlag();

    void setJamDetected(bool jammed);
    void clearJam();

private:
    int m_gateId{1};
    int m_actuationCount{0};
    bool m_wasActuated{false};
    bool m_isJammed{false};
    mutable Mutex m_mutex;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SIMULATEDGATE_HPP
