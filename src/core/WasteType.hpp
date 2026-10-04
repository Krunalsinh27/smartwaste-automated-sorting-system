#ifndef SMARTWASTE_CORE_WASTETYPE_HPP
#define SMARTWASTE_CORE_WASTETYPE_HPP

#include <string>

namespace smartwaste::core {

enum class WasteType {
    PLASTIC,
    METAL,
    PAPER,
    GLASS,
    OTHER,
    UNKNOWN
};

/**
 * @brief Converts a WasteType enum value to its corresponding string representation.
 * @param type The WasteType enum value.
 * @return std::string Human-readable name of the waste type.
 */
std::string wasteTypeToString(WasteType type);

/**
 * @brief Safely parses a string into a WasteType enum value.
 * @param str String representation of waste type (case-insensitive).
 * @return WasteType Corresponding enum value, or WasteType::UNKNOWN if unmapped.
 */
WasteType stringToWasteType(const std::string& str);

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_WASTETYPE_HPP
