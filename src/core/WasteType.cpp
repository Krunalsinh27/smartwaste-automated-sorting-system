#include "WasteType.hpp"
#include <algorithm>
#include <cctype>

namespace smartwaste::core {

std::string wasteTypeToString(WasteType type) {
    switch (type) {
        case WasteType::PLASTIC: return "PLASTIC";
        case WasteType::METAL:   return "METAL";
        case WasteType::PAPER:   return "PAPER";
        case WasteType::GLASS:   return "GLASS";
        case WasteType::OTHER:   return "OTHER";
        case WasteType::UNKNOWN:
        default:                 return "UNKNOWN";
    }
}

WasteType stringToWasteType(const std::string& str) {
    std::string upperStr;
    upperStr.reserve(str.size());
    for (char ch : str) {
        upperStr.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }

    if (upperStr == "PLASTIC") return WasteType::PLASTIC;
    if (upperStr == "METAL")   return WasteType::METAL;
    if (upperStr == "PAPER")   return WasteType::PAPER;
    if (upperStr == "GLASS")   return WasteType::GLASS;
    if (upperStr == "OTHER")   return WasteType::OTHER;

    return WasteType::UNKNOWN;
}

} // namespace smartwaste::core
