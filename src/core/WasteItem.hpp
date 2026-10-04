#ifndef SMARTWASTE_CORE_WASTEITEM_HPP
#define SMARTWASTE_CORE_WASTEITEM_HPP

#include "WasteType.hpp"
#include <string>
#include <cstdint>

namespace smartwaste::core {

/**
 * @brief Framework-independent bounding box structure.
 */
struct BoundingBox {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    int area() const noexcept { return width * height; }
};

/**
 * @brief Domain model representing a detected waste item.
 */
struct WasteItem {
    std::string id;
    std::int64_t timestampMs{0};
    WasteType type{WasteType::UNKNOWN};
    double confidence{0.0};
    BoundingBox boundingBox;
    int gateId{0};
    std::string sortingStatus{"PENDING"};

    std::string toString() const;
};

} // namespace smartwaste::core

#endif // SMARTWASTE_CORE_WASTEITEM_HPP
