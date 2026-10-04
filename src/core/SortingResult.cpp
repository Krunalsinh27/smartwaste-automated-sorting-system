#include "SortingResult.hpp"
#include <sstream>
#include <iomanip>

namespace smartwaste::core {

std::string SortingResult::toString() const {
    std::ostringstream oss;
    oss << "[SortingResult] Type: " << wasteTypeToString(type)
        << " -> Target Gate: " << targetGate
        << " | Conf: " << std::fixed << std::setprecision(2) << (confidence * 100.0) << "%"
        << " | Status: " << status;
    if (!message.empty()) {
        oss << " | Msg: " << message;
    }
    return oss.str();
}

} // namespace smartwaste::core
