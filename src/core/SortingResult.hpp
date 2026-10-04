#ifndef SMARTWASTE_CORE_SORTINGRESULT_HPP
#define SMARTWASTE_CORE_SORTINGRESULT_HPP

#include "WasteType.hpp"
#include <string>

namespace smartwaste::core {

/**
 * @brief Domain model representing the decision outcome for a sorted item.
 */
struct SortingResult {
    WasteType type{WasteType::UNKNOWN};
    int targetGate{0};
    double confidence{0.0};
    std::string status{"SUCCESS"}; // SUCCESS, REJECTED, GATE_UNAVAILABLE
    std::string message;

    std::string toString() const;
};

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_SORTINGRESULT_HPP
