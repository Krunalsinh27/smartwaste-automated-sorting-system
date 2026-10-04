#ifndef SMARTWASTE_HAL_IMOTOR_HPP
#define SMARTWASTE_HAL_IMOTOR_HPP

namespace smartwaste::hal {

/**
 * @brief Pure virtual interface for electric drive motor actuators.
 * Abstracts motor start, stop, and RPM telemetry.
 */
class IMotor {
public:
    virtual ~IMotor() = default;

    /**
     * @brief Engages motor power to begin rotation.
     */
    virtual void start() = 0;

    /**
     * @brief Cuts motor power to halt rotation.
     */
    virtual void stop() = 0;

    /**
     * @brief Gets current motor shaft rotational speed.
     * @return double Rotations per minute (RPM).
     */
    virtual double getRPM() const = 0;
};

} // namespace smartwaste::hal

#endif // SMARTWASTE_HAL_IMOTOR_HPP
