#ifndef SMARTWASTE_HAL_IGATE_HPP
#define SMARTWASTE_HAL_IGATE_HPP

namespace smartwaste::hal {

/**
 * @brief Pure virtual interface for pneumatic waste diverter gates.
 */
class IGate {
public:
    virtual ~IGate() = default;

    /**
     * @brief Triggers diverter gate actuation to route items off the conveyor belt.
     */
    virtual void actuate() = 0;

    /**
     * @brief Checks if mechanical obstruction or gate jam is detected.
     * @return true if jammed, false if operating normally.
     */
    virtual bool isJamDetected() const = 0;
};

} // namespace smartwaste::hal

#endif // SMARTWASTE_HAL_IGATE_HPP
