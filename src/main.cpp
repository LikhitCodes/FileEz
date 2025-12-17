#include <iostream>

// Core networking
#include "../include/network_utils.h"
#include "../include/app_config.h"
// Local HTTP control server
#include "control_server.h"

int main() {
    std::cout << "========================================\n";
    std::cout << "  P2P File Sharing System (Web UI Mode)\n";
    std::cout << "========================================\n";

    // Initialize network subsystem (Windows/Linux safe)
    if (!NetworkUtils::initializeNetwork()) {
        std::cerr << " Failed to initialize network subsystem\n";
        return 1;
    }

    std::cout << " Network initialized successfully\n";
    std::cout << " Starting local control server...\n";
    std::cout << " Open your browser / React app to control the node\n\n";

    startControlServer();

    // Cleanup (normally reached only on shutdown)
    NetworkUtils::cleanupNetwork();
    std::cout << " Network cleaned up\n";

    return 0;
}

// Stub implementations for functions called by control_server
void runShareMode(const AppConfig& config) {
    std::cout << "Share mode called - port: " << config.port << std::endl;
    // TODO: Implement share mode logic
}

void runDiscoverMode(const AppConfig& config) {
    std::cout << "Discover mode called" << std::endl;
    // TODO: Implement discover mode logic
}
