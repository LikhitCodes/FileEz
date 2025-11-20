#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <cstring>
#include <map>
#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
#endif
#include "../include/peer.h"
#include "../include/file_utils.h"
#include "../include/network_utils.h"
#include "../include/server.h"
#include "../include/client.h"

// Application modes
enum AppMode {
    MODE_SHARE,
    MODE_DOWNLOAD,
    MODE_HYBRID
};

// Global configuration
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
    
    AppConfig() : mode(MODE_HYBRID), port(DEFAULT_PORT), 
                  peersFile("peers.txt"),
                  sharedDir("data/shared"),
                  chunksDir("data/chunks"),
                  downloadDir("data/downloads"),
                  verbose(false),
                  runTests(false) {}
};

// Function declarations
void printUsage();
bool parseArguments(int argc, char* argv[], AppConfig& config);
void runShareMode(const AppConfig& config);
void runDownloadMode(const AppConfig& config);
void runHybridMode(const AppConfig& config);
void startServer(int port);
void startClient(const AppConfig& config);
void testNetworkFunctionality();
void testFileManagement();

int main(int argc, char* argv[]) {
    AppConfig config;
    
    // Initialize network subsystem
    if (!NetworkUtils::initializeNetwork()) {
        std::cerr << "Failed to initialize network subsystem" << std::endl;
        return 1;
    }
    
    // Parse command line arguments
    if (!parseArguments(argc, argv, config)) {
        printUsage();
        NetworkUtils::cleanupNetwork();
        return 1;
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  P2P File Sharing System" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Mode: " << (config.mode == MODE_SHARE ? "Share" : 
                             config.mode == MODE_DOWNLOAD ? "Download" : "Hybrid") << std::endl;
    std::cout << "Port: " << config.port << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Run tests if requested
    if (config.runTests) {
        testNetworkFunctionality();
        testFileManagement();
        NetworkUtils::cleanupNetwork();
        return 0;
    }
    
    try {
        switch (config.mode) {
            case MODE_SHARE:
                runShareMode(config);
                break;
            case MODE_DOWNLOAD:
                runDownloadMode(config);
                break;
            case MODE_HYBRID:
                runHybridMode(config);
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        NetworkUtils::cleanupNetwork();
        return 1;
    }
    
    NetworkUtils::cleanupNetwork();
    return 0;
}

void printUsage() {
    std::cout << "\n=== P2P File Sharing System ===\n\n"
              << "Usage: p2p_share [OPTIONS]\n\n"
              << "Options:\n"
              << "  --mode=<share|download|hybrid>  Operation mode (default: hybrid)\n"
              << "  --file=<filename>               File to share or download\n"
              << "  --port=<port>                   Port number (default: 8080)\n"
              << "  --peers=<filename>              Peers file (default: peers.txt)\n"
              << "  --verbose, -v                   Enable verbose output\n"
              << "  --test                          Run system tests\n"
              << "  --help, -h                      Show this help message\n\n"
              << "Examples:\n"
              << "  Share a file:\n"
              << "    p2p_share --mode=share --file=document.pdf --port=8080\n\n"
              << "  Download a file:\n"
              << "    p2p_share --mode=download --file=document.pdf\n\n"
              << "  Run in hybrid mode (share and download):\n"
              << "    p2p_share --mode=hybrid --port=8080\n\n";
}

bool parseArguments(int argc, char* argv[], AppConfig& config) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            return false;
        }
        else if (arg.find("--mode=") == 0) {
            std::string mode = arg.substr(7);
            if (mode == "share") {
                config.mode = MODE_SHARE;
            } else if (mode == "download") {
                config.mode = MODE_DOWNLOAD;
            } else if (mode == "hybrid") {
                config.mode = MODE_HYBRID;
            } else {
                std::cerr << "Invalid mode: " << mode << std::endl;
                return false;
            }
        }
        else if (arg.find("--file=") == 0) {
            config.filename = arg.substr(7);
        }
        else if (arg.find("--port=") == 0) {
            config.port = std::stoi(arg.substr(7));
        }
        else if (arg.find("--peers=") == 0) {
            config.peersFile = arg.substr(8);
        }
        else if (arg == "--verbose" || arg == "-v") {
            config.verbose = true;
        }
        else if (arg == "--test") {
            config.runTests = true;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return false;
        }
    }
    
    return true;
}

void runShareMode(const AppConfig& config) {
    std::cout << "\n=== Share Mode ===" << std::endl;
    
    // Check if file is specified
    if (config.filename.empty()) {
        std::cerr << "Error: No file specified. Use --file=<filename>" << std::endl;
        return;
    }
    
    std::string filePath = config.sharedDir + "/" + config.filename;
    
    // Check if file exists
    if (!FileUtils::fileExists(filePath)) {
        std::cerr << "Error: File not found: " << filePath << std::endl;
        return;
    }
    
    std::cout << "Sharing file: " << filePath << std::endl;
    
    // Split file into chunks
    std::vector<ChunkInfo> chunks;
    std::cout << "Splitting file into chunks..." << std::endl;
    if (!FileUtils::splitFileToChunks(filePath, config.chunksDir, chunks)) {
        std::cerr << "Error: Failed to split file into chunks" << std::endl;
        return;
    }
    
    std::cout << "File split into " << chunks.size() << " chunks" << std::endl;
    
    // Save chunk metadata
    std::string metaFile = config.chunksDir + "/" + config.filename + ".meta";
    FileUtils::saveChunkMetadata(chunks, metaFile);
    
    // Start server
    std::cout << "Starting P2P server on port " << config.port << "..." << std::endl;
    P2PServer server(config.port, config.chunksDir);
    
    if (!server.start()) {
        std::cerr << "Error: Failed to start server" << std::endl;
        return;
    }
    
    std::cout << "\nServer is running. Sharing " << chunks.size() << " chunks." << std::endl;
    std::cout << "Press Enter to stop server..." << std::endl;
    
    // Wait for user input
    std::cin.get();
    
    std::cout << "\nStopping server..." << std::endl;
    server.stop();
    std::cout << "Server stopped." << std::endl;
}

void runDownloadMode(const AppConfig& config) {
    std::cout << "\n=== Download Mode ===" << std::endl;
    
    // Check if file is specified
    if (config.filename.empty()) {
        std::cerr << "Error: No file specified. Use --file=<filename>" << std::endl;
        return;
    }
    
    std::cout << "Downloading file: " << config.filename << std::endl;
    
    // Create client
    P2PClient client(config.downloadDir, config.chunksDir + "_download");
    
    // Load peers
    if (!client.loadPeers(config.peersFile)) {
        std::cerr << "Error: Failed to load peers from " << config.peersFile << std::endl;
        return;
    }
    
    std::cout << "Loaded peers from " << config.peersFile << std::endl;
    
    // Download file
    std::cout << "Starting download..." << std::endl;
    if (client.downloadFile(config.filename)) {
        std::cout << "\n=== Download Complete ===" << std::endl;
        std::cout << "File saved to: " << config.downloadDir << "/" << config.filename << std::endl;
        
        // Verify file
        std::string downloadedFile = config.downloadDir + "/" + config.filename;
        size_t fileSize = FileUtils::getFileSize(downloadedFile);
        std::string hash = FileUtils::computeFileHash(downloadedFile);
        
        std::cout << "File size: " << fileSize << " bytes" << std::endl;
        std::cout << "File hash: " << hash << std::endl;
    } else {
        std::cerr << "\n=== Download Failed ===" << std::endl;
    }
}

void runHybridMode(const AppConfig& config) {
    std::cout << "\n=== Hybrid Mode ===" << std::endl;
    std::cout << "Running as both server and client" << std::endl;
    
    // Start server in background
    std::cout << "Starting P2P server on port " << config.port << "..." << std::endl;
    P2PServer server(config.port, config.chunksDir);
    
    if (!server.start()) {
        std::cerr << "Error: Failed to start server" << std::endl;
        return;
    }
    
    std::cout << "Server started successfully" << std::endl;
    
    // Create client
    P2PClient client(config.downloadDir, config.chunksDir + "_download");
    
    // Load peers
    if (client.loadPeers(config.peersFile)) {
        std::cout << "Loaded peers from " << config.peersFile << std::endl;
    }
    
    // Interactive menu
    bool running = true;
    while (running) {
        std::cout << "\n=== P2P File Sharing Menu ===" << std::endl;
        std::cout << "1. Share a file" << std::endl;
        std::cout << "2. Download a file" << std::endl;
        std::cout << "3. List available files" << std::endl;
        std::cout << "4. Show server status" << std::endl;
        std::cout << "5. Exit" << std::endl;
        std::cout << "Choose an option: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1: {
                std::cout << "Enter filename to share (in " << config.sharedDir << "): ";
                std::string filename;
                std::getline(std::cin, filename);
                
                std::string filePath = config.sharedDir + "/" + filename;
                if (FileUtils::fileExists(filePath)) {
                    std::vector<ChunkInfo> chunks;
                    if (FileUtils::splitFileToChunks(filePath, config.chunksDir, chunks)) {
                        std::cout << "File split into " << chunks.size() << " chunks and ready to share" << std::endl;
                        server.loadChunksFromDirectory();
                    }
                } else {
                    std::cout << "File not found: " << filePath << std::endl;
                }
                break;
            }
            
            case 2: {
                std::cout << "Enter filename to download: ";
                std::string filename;
                std::getline(std::cin, filename);
                
                if (client.downloadFile(filename)) {
                    std::cout << "Download complete: " << config.downloadDir << "/" << filename << std::endl;
                } else {
                    std::cout << "Download failed" << std::endl;
                }
                break;
            }
            
            case 3: {
                std::cout << "\nAvailable files on server:" << std::endl;
                std::vector<ChunkInfo> chunks = server.getAvailableChunks();
                
                // Group by filename
                std::map<std::string, int> fileChunks;
                for (const auto& chunk : chunks) {
                    fileChunks[chunk.filename]++;
                }
                
                if (fileChunks.empty()) {
                    std::cout << "  No files available" << std::endl;
                } else {
                    for (const auto& file : fileChunks) {
                        std::cout << "  - " << file.first << " (" << file.second << " chunks)" << std::endl;
                    }
                }
                break;
            }
            
            case 4: {
                std::cout << "\nServer Status:" << std::endl;
                std::cout << "  Port: " << config.port << std::endl;
                std::cout << "  Running: " << (server.isRunning() ? "Yes" : "No") << std::endl;
                std::cout << "  Available chunks: " << server.getAvailableChunks().size() << std::endl;
                break;
            }
            
            case 5: {
                running = false;
                break;
            }
            
            default:
                std::cout << "Invalid option" << std::endl;
                break;
        }
    }
    
    std::cout << "\nStopping server..." << std::endl;
    server.stop();
    std::cout << "Goodbye!" << std::endl;
}

void startServer(int port) {
    std::cout << "Starting standalone server on port " << port << std::endl;
    P2PServer server(port);
    
    if (server.start()) {
        std::cout << "Server running. Press Enter to stop..." << std::endl;
        std::cin.get();
        server.stop();
    }
}

void startClient(const AppConfig& config) {
    std::cout << "Starting standalone client" << std::endl;
    P2PClient client(config.downloadDir, config.chunksDir);
    
    if (client.loadPeers(config.peersFile)) {
        std::cout << "Client ready" << std::endl;
    }
}

void testFileManagement() {
    std::cout << "\n=== Testing File Management ===" << std::endl;
    
    // Test 1: Create a test file
    std::string testFile = "data/shared/phase3_test.txt";
    std::ofstream file(testFile.c_str());
    file << "This is a test file for Phase 3 file management.\n";
    file << "It demonstrates file chunking and merging capabilities.\n";
    file << "The P2P system will split this into chunks and reassemble it.\n";
    file.close();
    std::cout << "Created test file: " << testFile << std::endl;
    
    // Test 2: Get file info
    size_t fileSize = FileUtils::getFileSize(testFile);
    std::cout << "File size: " << fileSize << " bytes" << std::endl;
    
    // Test 3: Compute hash
    std::string hash = FileUtils::computeFileHash(testFile);
    std::cout << "File hash: " << hash << std::endl;
    
    // Test 4: Split file into chunks
    std::vector<ChunkInfo> chunks;
    if (FileUtils::splitFileToChunks(testFile, "data/chunks", chunks)) {
        std::cout << "Split into " << chunks.size() << " chunks" << std::endl;
        
        // Test 5: Verify chunks
        for (const auto& chunk : chunks) {
            std::string chunkPath = FileUtils::getChunkPath("data/chunks", chunk.filename, chunk.chunkIndex);
            if (FileUtils::isChunkComplete(chunkPath, chunk)) {
                std::cout << "  Chunk " << chunk.chunkIndex << " verified (" << chunk.chunkSize << " bytes)" << std::endl;
            }
        }
        
        // Test 6: Merge chunks back
        std::string mergedFile = "data/downloads/phase3_merged.txt";
        if (FileUtils::mergeChunksFromInfo(chunks, "data/chunks", mergedFile)) {
            std::cout << "Merged chunks into: " << mergedFile << std::endl;
            
            // Test 7: Compare files
            if (FileUtils::compareFiles(testFile, mergedFile)) {
                std::cout << "SUCCESS: Merged file matches original!" << std::endl;
            } else {
                std::cout << "ERROR: Merged file differs from original" << std::endl;
            }
        }
        
        // Test 8: Save chunk metadata
        if (FileUtils::saveChunkMetadata(chunks, "data/chunks/phase3_test.meta")) {
            std::cout << "Saved chunk metadata" << std::endl;
        }
    }
    
    std::cout << "=== File Management Tests Complete ===" << std::endl;
}

void testNetworkFunctionality() {
    std::cout << "\n=== Testing Network Functionality ===" << std::endl;
    
    // Test 1: Check local IP
    std::string localIP = NetworkUtils::getLocalIP();
    std::cout << "Local IP: " << localIP << std::endl;
    
    // Test 2: Validate IP addresses
    std::cout << "Is '127.0.0.1' valid? " << (NetworkUtils::isValidIP("127.0.0.1") ? "Yes" : "No") << std::endl;
    std::cout << "Is '999.999.999.999' valid? " << (NetworkUtils::isValidIP("999.999.999.999") ? "Yes" : "No") << std::endl;
    
    // Test 3: Check port availability
    std::cout << "Is port 8080 available? " << (NetworkUtils::isPortAvailable(8080) ? "Yes" : "No") << std::endl;
    std::cout << "Is port 8081 available? " << (NetworkUtils::isPortAvailable(8081) ? "Yes" : "No") << std::endl;
    
    // Test 4: Load peers from file
    PeerManager peerManager;
    if (peerManager.loadPeersFromFile("peers.txt")) {
        std::vector<Peer> peers = peerManager.getActivePeers();
        std::cout << "Loaded " << peers.size() << " peers" << std::endl;
    }
    
    std::cout << "=== Network Tests Complete ===" << std::endl;
}