#include "config/ConfigManager.hpp"
#include "logging/Logger.hpp"
#include "engine/SmartWasteEngine.hpp"
#include "api/HttpServer.hpp"

#include <iostream>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOGDI
#define NOGDI
#endif
#include <windows.h>
static volatile bool g_running = true;

BOOL WINAPI consoleCtrlHandler(DWORD ctrlType) {
    if (ctrlType == CTRL_C_EVENT || ctrlType == CTRL_CLOSE_EVENT || ctrlType == CTRL_BREAK_EVENT) {
        g_running = false;
        return TRUE;
    }
    return FALSE;
}
#else
#include <csignal>
#include <unistd.h>
static volatile sig_atomic_t g_running = 1;

void signalHandler(int signum) {
    (void)signum;
    g_running = 0;
}
#endif

int main() {
    // 1. Setup Ctrl+C signal handler for graceful shutdown
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
#else
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
#endif

    // 2. Initialize Logger
    smartwaste::logging::Logger& logger = smartwaste::logging::Logger::getInstance();
    logger.setLogLevel(smartwaste::logging::LogLevel::LOG_DEBUG_LEVEL);
    logger.setLogFile("data/smartwaste_server.log");

    LOG_INFO("Server", "==================================================");
    LOG_INFO("Server", "SmartWaste Dedicated REST API Server Startup");
    LOG_INFO("Server", "==================================================");

    // 3. Load System Configuration
    smartwaste::config::ConfigManager configMgr;
    std::string configFile = "config/config.json";
    try {
        configMgr.loadFromFile(configFile);
        LOG_INFO("Server", "Loaded runtime configuration from " + configFile);
    } catch (const smartwaste::config::ConfigException& ex) {
        LOG_ERROR("Server", std::string("Configuration loading failed: ") + ex.what());
        std::cerr << "Configuration loading failed: " << ex.what() << std::endl;
        return 1;
    }

    // 4. Initialize Core SmartWaste Engine
    smartwaste::engine::SmartWasteEngine engine("data/events.jsonl");
    if (!engine.initialize(configFile)) {
        LOG_ERROR("Server", "Failed to initialize SmartWasteEngine!");
        std::cerr << "Failed to initialize SmartWasteEngine!" << std::endl;
        return 1;
    }
    LOG_INFO("Server", "SmartWasteEngine initialized successfully.");

    // 5. Initialize Native WinSock2 HTTP Server
    smartwaste::api::ServerConfig serverConfig{"127.0.0.1", 8080, 1048576, 10};
    smartwaste::api::HttpServer server(serverConfig);

    // Wire Engine to ApiRouter
    server.getRouter().setEngine(&engine);

    if (!server.start()) {
        LOG_ERROR("Server", "Failed to start HTTP server on 127.0.0.1:8080. Port may be in use.");
        std::cerr << "Failed to start HTTP server on 127.0.0.1:8080" << std::endl;
        return 1;
    }

    // 6. Display Server Status Banner
    std::cout << "\n==================================================" << std::endl;
    std::cout << "SmartWaste Server" << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << "Host:   127.0.0.1" << std::endl;
    std::cout << "Port:   8080" << std::endl;
    std::cout << "API:    http://127.0.0.1:8080" << std::endl;
    std::cout << "Status: RUNNING" << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    std::cout << "==================================================\n" << std::endl;

    LOG_INFO("Server", "SmartWaste Server RUNNING on http://127.0.0.1:8080");

    // 7. Continuous Server Event Loop
    while (g_running) {
        server.processPendingConnection();
#if defined(_WIN32) || defined(_WIN64)
        Sleep(5);
#else
        usleep(5000);
#endif
    }

    // 8. Graceful Shutdown
    std::cout << "\nShutting down SmartWaste Server..." << std::endl;
    LOG_INFO("Server", "Shutting down SmartWaste Server...");
    server.stop();
    engine.stopMachine();

    LOG_INFO("Server", "SmartWaste Server stopped cleanly.");
    std::cout << "SmartWaste Server stopped cleanly." << std::endl;

    return 0;
}
