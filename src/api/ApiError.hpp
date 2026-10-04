#ifndef SMARTWASTE_API_APIERROR_HPP
#define SMARTWASTE_API_APIERROR_HPP

#include "../config/json.hpp"
#include <string>

namespace smartwaste::api {

using json = nlohmann::json;

/**
 * @brief Helper functions to create standardized JSON error response payloads.
 */
class ApiError {
public:
    static json createErrorJson(const std::string& code, const std::string& message) {
        json j;
        j["success"] = false;
        j["error"] = {
            {"code", code},
            {"message", message}
        };
        return j;
    }

    static json badRequest(const std::string& message = "Malformed HTTP request or invalid payload.") {
        return createErrorJson("BAD_REQUEST", message);
    }

    static json notFound(const std::string& message = "The requested API endpoint was not found.") {
        return createErrorJson("NOT_FOUND", message);
    }

    static json methodNotAllowed(const std::string& message = "HTTP method not allowed for this endpoint.") {
        return createErrorJson("METHOD_NOT_ALLOWED", message);
    }

    static json payloadTooLarge(const std::string& message = "Request body exceeds maximum allowed size.") {
        return createErrorJson("PAYLOAD_TOO_LARGE", message);
    }

    static json invalidStateTransition(const std::string& message = "Invalid machine state transition.") {
        return createErrorJson("INVALID_STATE_TRANSITION", message);
    }

    static json internalServerError(const std::string& message = "An internal server error occurred.") {
        return createErrorJson("INTERNAL_SERVER_ERROR", message);
    }
};

} // namespace smartwaste::api

#endif // SMARTWASTE_API_APIERROR_HPP
