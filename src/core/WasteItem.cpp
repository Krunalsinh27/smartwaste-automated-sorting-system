#include "WasteItem.hpp"
#include <sstream>
#include <iomanip>

namespace smartwaste::core {

std::string WasteItem::toString() const {
    std::ostringstream oss;
    oss << "[WasteItem ID: " << id << "]"
        << " Type: " << wasteTypeToString(type)
        << " | Conf: " << std::fixed << std::setprecision(2) << (confidence * 100.0) << "%"
        << " | BBox: (" << boundingBox.x << ", " << boundingBox.y << ", "
        << boundingBox.width << "x" << boundingBox.height << ")"
        << " | Gate: " << gateId
        << " | Status: " << sortingStatus;
    return oss.str();
}

} // namespace smartwaste::core
