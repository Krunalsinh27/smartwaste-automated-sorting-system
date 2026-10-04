#ifndef SMARTWASTE_HAL_ISENSOR_HPP
#define SMARTWASTE_HAL_ISENSOR_HPP

namespace smartwaste::hal {

/**
 * @brief Pure virtual interface for industrial detection sensors (e.g., optical, proximity, weight).
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * @brief Reads raw analog or digital sensor signal value.
     * @return double Sensor signal level.
     */
    virtual double readSignal() const = 0;

    /**
     * @brief Checks if sensor detection threshold is currently triggered.
     * @return true if an item/event is detected, false otherwise.
     */
    virtual bool isTriggered() const = 0;
};

} // namespace smartwaste::hal

#endif // SMARTWASTE_HAL_ISENSOR_HPP
