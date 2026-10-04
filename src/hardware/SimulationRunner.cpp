#include "SimulationRunner.hpp"
#include "HardwareManager.hpp"
#include "HardwareTelemetryAggregator.hpp"
#include "SystemHealthMonitor.hpp"
#include "../core/EventHistory.hpp"
#include "../controller/SortingController.hpp"
#include "../logging/Logger.hpp"

#include <sstream>

namespace smartwaste::hardware {

std::string SimulationStatistics::toString() const {
    std::ostringstream oss;
    oss << "--- Simulation Statistics ---\n\n"
        << "Total Items: " << totalItems << "\n"
        << "Successful: " << successfulItems << "\n"
        << "Rejected: " << rejectedItems << "\n"
        << "Gate Unavailable: " << unavailableGateItems << "\n"
        << "Failed: " << failedItems << "\n"
        << "Successful Gate Actuations: " << totalGateActuations;

    if (healthChecksPerformed > 0) {
        oss << "\n\n--- Health Summary ---\n\n"
            << "Health Checks: " << healthChecksPerformed << "\n"
            << "Healthy: " << healthyChecks << "\n"
            << "Warnings: " << warningChecks << "\n"
            << "Critical/Fault: " << criticalChecks;
    }
    return oss.str();
}

SimulationRunner::SimulationRunner(std::chrono::milliseconds tickSize)
    : m_tickSize(tickSize.count() > 0 ? tickSize : std::chrono::milliseconds(10)),
      m_elapsedTime(0),
      m_tickCount(0) {}

std::uint64_t SimulationRunner::stepSimulation(std::chrono::milliseconds duration) {
    if (duration.count() <= 0) {
        return m_tickCount;
    }

    std::uint64_t newTicks = duration.count() / m_tickSize.count();
    if (newTicks == 0) {
        newTicks = 1; // Minimum 1 tick step
    }

    m_tickCount += newTicks;
    m_elapsedTime += duration;

    std::ostringstream oss;
    oss << "[SimulationRunner] Advanced simulation by " << duration.count()
        << " ms (" << newTicks << " ticks). Total Elapsed: " << m_elapsedTime.count()
        << " ms | Total Ticks: " << m_tickCount;
    LOG_INFO("Hardware", oss.str());

    return m_tickCount;
}

smartwaste::core::SortingResult SimulationRunner::processItem(
    const smartwaste::core::WasteItem& item,
    const smartwaste::controller::SortingController& controller,
    HardwareManager& hwManager,
    const HardwareTelemetryAggregator* aggregator,
    const SystemHealthMonitor* healthMonitor,
    smartwaste::core::EventHistory* eventHistory
) {
    m_stats.totalItems++;
    stepSimulation(std::chrono::milliseconds(100));

    std::ostringstream ossStart;
    ossStart << "\nProcessing " << item.id
             << "\nType: " << smartwaste::core::wasteTypeToString(item.type)
             << "\nConfidence: " << item.confidence;
    LOG_INFO("Simulation", ossStart.str());

    if (eventHistory != nullptr) {
        eventHistory->addEvent(
            smartwaste::core::EventType::ITEM_DETECTED,
            smartwaste::logging::LogLevel::LOG_INFO_LEVEL,
            "Waste item detected on conveyor belt.",
            item.id,
            item.type
        );
    }

    smartwaste::core::SortingResult result = controller.processAndRoute(item, hwManager);

    if (eventHistory != nullptr) {
        eventHistory->addEvent(
            smartwaste::core::EventType::ITEM_CLASSIFIED,
            smartwaste::logging::LogLevel::LOG_INFO_LEVEL,
            "Waste item classified as " + smartwaste::core::wasteTypeToString(item.type) +
                " (Confidence: " + std::to_string(static_cast<int>(item.confidence * 100)) + "%).",
            item.id,
            item.type,
            result.targetGate
        );
    }

    if (result.status == "SUCCESS") {
        m_stats.successfulItems++;
        m_stats.totalGateActuations++;
        std::ostringstream ossRes;
        ossRes << "Target Gate: " << result.targetGate
               << "\nResult: SUCCESS";
        LOG_INFO("Simulation", ossRes.str());

        if (eventHistory != nullptr) {
            eventHistory->addEvent(
                smartwaste::core::EventType::GATE_ACTUATED,
                smartwaste::logging::LogLevel::LOG_INFO_LEVEL,
                "Pneumatic diverter gate #" + std::to_string(result.targetGate) + " actuated cleanly.",
                item.id,
                item.type,
                result.targetGate
            );
            eventHistory->addEvent(
                smartwaste::core::EventType::ITEM_SORTED,
                smartwaste::logging::LogLevel::LOG_INFO_LEVEL,
                "Item successfully sorted into Gate #" + std::to_string(result.targetGate) + ".",
                item.id,
                item.type,
                result.targetGate
            );
        }
    } else if (result.status == "REJECTED") {
        m_stats.rejectedItems++;
        std::ostringstream ossRes;
        ossRes << "Result: REJECTED";
        LOG_WARN("Simulation", ossRes.str());

        if (eventHistory != nullptr) {
            eventHistory->addEvent(
                smartwaste::core::EventType::ITEM_REJECTED,
                smartwaste::logging::LogLevel::LOG_WARN_LEVEL,
                "Item rejected: Confidence (" + std::to_string(static_cast<int>(item.confidence * 100)) +
                    "%) is below threshold.",
                item.id,
                item.type
            );
        }
    } else if (result.status == "GATE_UNAVAILABLE") {
        m_stats.unavailableGateItems++;
        std::ostringstream ossRes;
        ossRes << "Target Gate: " << result.targetGate
               << "\nResult: GATE_UNAVAILABLE";
        LOG_WARN("Simulation", ossRes.str());

        if (eventHistory != nullptr) {
            eventHistory->addEvent(
                smartwaste::core::EventType::GATE_UNAVAILABLE,
                smartwaste::logging::LogLevel::LOG_WARN_LEVEL,
                "Target Gate #" + std::to_string(result.targetGate) + " is not registered or unavailable.",
                item.id,
                item.type,
                result.targetGate
            );
        }
    } else {
        m_stats.failedItems++;
        std::ostringstream ossRes;
        ossRes << "Target Gate: " << result.targetGate
               << "\nResult: FAILED (" << result.status << ")";
        LOG_ERROR("Simulation", ossRes.str());

        if (eventHistory != nullptr) {
            eventHistory->addEvent(
                smartwaste::core::EventType::GATE_JAMMED,
                smartwaste::logging::LogLevel::LOG_ERROR_LEVEL,
                "Gate #" + std::to_string(result.targetGate) + " failed actuation due to mechanical jam!",
                item.id,
                item.type,
                result.targetGate
            );
        }
    }

    // Telemetry & Health Monitoring Integration (Phase 4 Step 4 & 5)
    if (aggregator != nullptr) {
        HardwareTelemetry telemetry = aggregator->read();
        LOG_INFO("Telemetry", telemetry.toString());

        if (healthMonitor != nullptr) {
            HardwareHealth health = healthMonitor->evaluate(telemetry);
            LOG_INFO("Health", health.toString());

            m_stats.healthChecksPerformed++;
            if (health.status == HealthStatus::HEALTHY) {
                m_stats.healthyChecks++;
            } else if (health.status == HealthStatus::WARNING) {
                m_stats.warningChecks++;
                if (eventHistory != nullptr) {
                    eventHistory->addEvent(
                        smartwaste::core::EventType::HEALTH_WARNING,
                        smartwaste::logging::LogLevel::LOG_WARN_LEVEL,
                        health.message,
                        item.id
                    );
                }
            } else if (health.status == HealthStatus::FAULT) {
                m_stats.criticalChecks++;
                if (eventHistory != nullptr) {
                    eventHistory->addEvent(
                        smartwaste::core::EventType::HEALTH_FAULT,
                        smartwaste::logging::LogLevel::LOG_ERROR_LEVEL,
                        health.message,
                        item.id
                    );
                }
            }
        }
    }

    return result;
}

SimulationStatistics SimulationRunner::runSimulation(
    const std::vector<smartwaste::core::WasteItem>& items,
    const smartwaste::controller::SortingController& controller,
    HardwareManager& hwManager,
    const HardwareTelemetryAggregator* aggregator,
    const SystemHealthMonitor* healthMonitor,
    smartwaste::core::EventHistory* eventHistory
) {
    LOG_INFO("Simulation", "Starting automated simulation batch processing (" + std::to_string(items.size()) + " items)...");
    for (const auto& item : items) {
        processItem(item, controller, hwManager, aggregator, healthMonitor, eventHistory);
    }
    LOG_INFO("Simulation", "Batch simulation complete.");
    return m_stats;
}

void SimulationRunner::reset() {
    m_elapsedTime = std::chrono::milliseconds(0);
    m_tickCount = 0;
    m_stats.reset();
    LOG_INFO("Hardware", "[SimulationRunner] Simulation runner reset to 0 ms and zero statistics.");
}

} // namespace smartwaste::hardware
