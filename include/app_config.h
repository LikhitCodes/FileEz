#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <string>
#include "peer.h"  // For DEFAULT_PORT constant

// Application modes
enum AppMode {
    MODE_SHARE,
    MODE_DOWNLOAD,
    MODE_HYBRID,
    MODE_DISCOVER
};

struct AppConfig {
    AppMode mode;
    std::string filename;
    int port;
    std::string peersFile;
    std::string sharedDir;
    std::string chunksDir;
    std::string downloadDir;
    bool verbose;
    bool runTests;

    AppConfig()
        : mode(MODE_HYBRID),
          port(DEFAULT_PORT),
          peersFile("peers.txt"),
          sharedDir("data/shared"),
          chunksDir("data/chunks"),
          downloadDir("data/downloads"),
          verbose(false),
          runTests(false) {}
};

#endif // APP_CONFIG_H
