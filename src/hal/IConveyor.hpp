#ifndef SMARTWASTE_HAL_ICONVEYOR_HPP
#define SMARTWASTE_HAL_ICONVEYOR_HPP

namespace smartwaste::hal {

/**
 * @brief Pure virtual interface for industrial conveyor belt hardware.
 * Decouples conveyor speed control and stopping mechanics from physical drivers.
 */
class IConveyor {
public:
    virtual ~IConveyor() = default;

    /**
     * @brief Sets the target conveyor belt movement speed.
     * @param speed Target speed in meters per second (m/s).
     */
    virtual void setSpeed(double speed) = 0;

    /**
     * @brief Gets the current operational conveyor speed.
     * @return double Current speed in meters per second (m/s).
     */
    virtual double getSpeed() const = 0;

    /**
     * @brief Halts conveyor belt movement immediately.
     */
    virtual void stop() = 0;
};

} // namespace smartwaste::hal

#endif // SMARTWASTE_HAL_ICONVEYOR_HPP
