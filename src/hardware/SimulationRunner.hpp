#ifndef SMARTWASTE_HARDWARE_SIMULATIONRUNNER_HPP
#define SMARTWASTE_HARDWARE_SIMULATIONRUNNER_HPP

#include "../core/WasteItem.hpp"
#include "../core/SortingResult.hpp"

#include <chrono>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace smartwaste::core {
    class EventHistory;
}

namespace smartwaste::controller {
    class SortingController;
}

namespace smartwaste::hardware {

class HardwareManager;
class HardwareTelemetryAggregator;
class SystemHealthMonitor;

/**
 * @brief Statistics model capturing performance metrics during simulation runs.
 */
struct SimulationStatistics {
    std::size_t totalItems{0};
    std::size_t successfulItems{0};
    std::size_t rejectedItems{0};
    std::size_t unavailableGateItems{0};
    std::size_t failedItems{0};
    std::size_t totalGateActuations{0};

    // Phase 4 Step 4 Health Monitoring metrics
    std::size_t healthChecksPerformed{0};
    std::size_t healthyChecks{0};
    std::size_t warningChecks{0};
    std::size_t criticalChecks{0};

    void reset() noexcept {
        totalItems = 0;
        successfulItems = 0;
        rejectedItems = 0;
        unavailableGateItems = 0;
        failedItems = 0;
        totalGateActuations = 0;
        healthChecksPerformed = 0;
        healthyChecks = 0;
        warningChecks = 0;
        criticalChecks = 0;
    }

    std::string toString() const;
};

/**
 * @brief Deterministic simulation time progression and automated sorting pipeline runner.
 * Tracks elapsed simulation time, step tick counts, telemetry aggregation, health evaluation, and batch processing.
 */
class SimulationRunner {
public:
    explicit SimulationRunner(std::chrono::milliseconds tickSize = std::chrono::milliseconds(10));

    /**
     * @brief Steps simulation forward by specified duration.
     * @param duration Amount of simulation time to advance.
     * @return std::uint64_t Total simulation ticks executed.
     */
    std::uint64_t stepSimulation(std::chrono::milliseconds duration);

    /**
     * @brief Processes a single WasteItem through SortingController, HardwareManager, Telemetry, Health monitoring, and EventHistory.
     */
    smartwaste::core::SortingResult processItem(
        const smartwaste::core::WasteItem& item,
        const smartwaste::controller::SortingController& controller,
        HardwareManager& hwManager,
        const HardwareTelemetryAggregator* aggregator = nullptr,
        const SystemHealthMonitor* healthMonitor = nullptr,
        smartwaste::core::EventHistory* eventHistory = nullptr
    );

    /**
     * @brief Processes a vector sequence of WasteItems sequentially through the sorting & monitoring pipeline.
     */
    SimulationStatistics runSimulation(
        const std::vector<smartwaste::core::WasteItem>& items,
        const smartwaste::controller::SortingController& controller,
        HardwareManager& hwManager,
        const HardwareTelemetryAggregator* aggregator = nullptr,
        const SystemHealthMonitor* healthMonitor = nullptr,
        smartwaste::core::EventHistory* eventHistory = nullptr
    );

    /**
     * @brief Resets simulation clock, tick counters, and statistics metrics.
     */
    void reset();

    std::chrono::milliseconds getElapsedTime() const noexcept { return m_elapsedTime; }
    std::uint64_t getTickCount() const noexcept { return m_tickCount; }
    std::chrono::milliseconds getTickSize() const noexcept { return m_tickSize; }

    const SimulationStatistics& getStatistics() const noexcept { return m_stats; }

private:
    std::chrono::milliseconds m_tickSize{10};
    std::chrono::milliseconds m_elapsedTime{0};
    std::uint64_t m_tickCount{0};
    SimulationStatistics m_stats;
};

} // namespace smartwaste::hardware

#endif // SMARTWASTE_HARDWARE_SIMULATIONRUNNER_HPP
