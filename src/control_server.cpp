#include <iostream>
#include <memory>
#include <mutex>
#include <fstream>
#include <string>

// HTTP server (single-header library)
#include "../include/httplib.h"

// App configuration (MOVED FROM main.cpp)
#include "../include/app_config.h"

// Your existing includes
#include "../include/server.h"
#include "../include/client.h"
#include "../include/network_utils.h"
#include "../include/file_utils.h"
#include "../include/peer.h"

// Control server header
#include "../include/control_server.h"

// ---- functions defined elsewhere ----
void runShareMode(const AppConfig& config);
void runDownloadMode(const AppConfig& config);
void runDiscoverMode(const AppConfig& config);

// ---- global persistent state ----
static std::unique_ptr<P2PServer> g_server;
static std::unique_ptr<P2PClient> g_client;
static AppConfig g_config;
static std::mutex g_mutex;

// ---- init config once ----
static void initDefaultConfig() {
    static bool initialized = false;
    if (initialized) return;

    g_config.mode = MODE_HYBRID;
    g_config.port = 8081;  // Use different port for P2P server
    g_config.peersFile = "peers.txt";
    g_config.sharedDir = "data/shared";
    g_config.chunksDir = "data/chunks";
    g_config.downloadDir = "data/downloads";
    g_config.verbose = false;
    g_config.runTests = false;

    initialized = true;
}

// ---- control server ----
void startControlServer() {
    initDefaultConfig();

    httplib::Server http;

    // Enable CORS for frontend access
    http.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT"},
        {"Access-Control-Allow-Headers", "Content-Type, X-Filename, X-Requested-With"}
    });

    // Handle OPTIONS preflight requests
    http.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        // Don't set CORS headers here since they're already set as default headers
        res.status = 204;
    });

    std::cout << "[CONTROL] Listening on http://127.0.0.1:8080\n";

    http.Post("/server/start", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (!g_server) {
            g_server = std::make_unique<P2PServer>(
                g_config.port,
                g_config.chunksDir
            );

            if (!g_server->start()) {
                res.status = 500;
                res.set_content("Failed to start P2P server", "text/plain");
                return;
            }
        }

        res.set_content("P2P server started", "text/plain");
    });

    http.Post("/server/stop", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (g_server) {
            g_server->stop();
            g_server.reset();
        }

        res.set_content("P2P server stopped", "text/plain");
    });

    http.Post("/share", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (req.body.empty()) {
            res.status = 400;
            res.set_content("Filename required", "text/plain");
            return;
        }

        g_config.filename = req.body;
        g_config.mode = MODE_SHARE;

        runShareMode(g_config);

        res.set_content("File shared: " + req.body, "text/plain");
    });

    http.Post("/download", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (req.body.empty()) {
            res.status = 400;
            res.set_content("Filename required", "text/plain");
            return;
        }

        if (!g_client) {
            g_client = std::make_unique<P2PClient>(
                g_config.downloadDir,
                g_config.chunksDir + "_download"
            );
            g_client->loadPeers(g_config.peersFile);
        }

        bool ok = g_client->downloadFile(req.body);
        if (!ok) {
            res.status = 500;
            res.set_content("Download failed", "text/plain");
            return;
        }

        res.set_content("Download started: " + req.body, "text/plain");
    });

    http.Post("/discover", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_config.mode = MODE_DISCOVER;
        runDiscoverMode(g_config);

        res.set_content("Peer discovery completed", "text/plain");
    });

    http.Get("/status", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::string status = "{\n";
        status += "  \"server_running\": ";
        status += (g_server && g_server->isRunning()) ? "true" : "false";
        status += ",\n";

        status += "  \"available_chunks\": ";
        status += g_server ? std::to_string(g_server->getAvailableChunks().size()) : "0";
        status += "\n}\n";

        res.set_content(status, "application/json");
    });

    http.Get("/peers", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::ifstream file(g_config.peersFile);
        if (!file.is_open()) {
            res.set_content("[]", "application/json");
            return;
        }

        std::string json = "[\n";
        std::string line;
        bool first = true;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string ip = line.substr(0, colonPos);
            std::string port = line.substr(colonPos + 1);

            if (!first) json += ",\n";
            json += "  {\"ip\": \"" + ip + "\", \"port\": " + port + "}";
            first = false;
        }

        json += "\n]";
        res.set_content(json, "application/json");
    });

    http.Post("/peers/add", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (req.body.empty()) {
            res.status = 400;
            res.set_content("{\"error\": \"Request body required\"}", "application/json");
            return;
        }

        // Parse JSON manually (simple format: {"ip":"x.x.x.x","port":8080})
        std::string body = req.body;
        size_t ipStart = body.find("\"ip\"");
        size_t portStart = body.find("\"port\"");

        if (ipStart == std::string::npos || portStart == std::string::npos) {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid JSON format\"}", "application/json");
            return;
        }

        // Extract IP
        size_t ipValueStart = body.find("\"", ipStart + 5);
        size_t ipValueEnd = body.find("\"", ipValueStart + 1);
        std::string ip = body.substr(ipValueStart + 1, ipValueEnd - ipValueStart - 1);

        // Extract port
        size_t portValueStart = body.find(":", portStart + 6);
        size_t portValueEnd = body.find_first_of(",}", portValueStart + 1);
        std::string portStr = body.substr(portValueStart + 1, portValueEnd - portValueStart - 1);
        
        // Remove whitespace
        portStr.erase(0, portStr.find_first_not_of(" \t\n\r"));
        portStr.erase(portStr.find_last_not_of(" \t\n\r") + 1);

        // Validate
        if (ip.empty() || portStr.empty()) {
            res.status = 400;
            res.set_content("{\"error\": \"IP and port required\"}", "application/json");
            return;
        }

        // Append to peers.txt
        std::ofstream file(g_config.peersFile, std::ios::app);
        if (!file.is_open()) {
            res.status = 500;
            res.set_content("{\"error\": \"Failed to open peers file\"}", "application/json");
            return;
        }

        file << ip << ":" << portStr << "\n";
        file.close();

        res.set_content("{\"success\": true, \"message\": \"Peer added\"}", "application/json");
    });

    http.Get("/files/browse", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (!g_client) {
            g_client = std::make_unique<P2PClient>(
                g_config.downloadDir,
                g_config.chunksDir + "_download"
            );
            g_client->loadPeers(g_config.peersFile);
        }

        // Load peers from file
        std::ifstream file(g_config.peersFile);
        if (!file.is_open()) {
            res.set_content("{\"error\": \"No peers configured\"}", "application/json");
            return;
        }

        std::string json = "{\n  \"peers\": [\n";
        std::string line;
        bool firstPeer = true;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string ip = line.substr(0, colonPos);
            std::string portStr = line.substr(colonPos + 1);
            int port = std::stoi(portStr);

            // Get chunk list from this peer
            std::vector<ChunkInfo> chunks = g_client->requestChunkListFromPeer(ip, port);

            if (!firstPeer) json += ",\n";
            json += "    {\n";
            json += "      \"ip\": \"" + ip + "\",\n";
            json += "      \"port\": " + portStr + ",\n";
            json += "      \"files\": [\n";

            bool firstFile = true;
            for (const auto& chunk : chunks) {
                if (!firstFile) json += ",\n";
                json += "        {\n";
                json += "          \"filename\": \"" + chunk.filename + "\",\n";
                json += "          \"chunkIndex\": " + std::to_string(chunk.chunkIndex) + ",\n";
                json += "          \"chunkSize\": " + std::to_string(chunk.chunkSize) + ",\n";
                json += "          \"checksum\": \"" + chunk.checksum + "\"\n";
                json += "        }";
                firstFile = false;
            }

            json += "\n      ]\n";
            json += "    }";
            firstPeer = false;
        }

        json += "\n  ]\n}";
        res.set_content(json, "application/json");
    });

    http.Post("/peers/discover", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        g_config.mode = MODE_DISCOVER;
        runDiscoverMode(g_config);

        res.set_content("{\"success\": true, \"message\": \"Network discovery completed\"}", "application/json");
    });

    http.Post("/files/upload", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::cout << "[UPLOAD] Received upload request" << std::endl;
        std::cout << "[UPLOAD] Content-Type: " << req.get_header_value("Content-Type") << std::endl;
        std::cout << "[UPLOAD] Body size: " << req.body.size() << std::endl;
        
        // Print all headers for debugging
        for (const auto& header : req.headers) {
            std::cout << "[UPLOAD] Header: " << header.first << " = " << header.second << std::endl;
        }

        if (req.body.empty()) {
            std::cout << "[UPLOAD] Error: Empty file data" << std::endl;
            res.status = 400;
            res.set_content("{\"error\": \"File data required\"}", "application/json");
            return;
        }

        // Get filename from X-Filename header
        std::string filename = "uploaded_file";
        auto it = req.headers.find("X-Filename");
        if (it != req.headers.end()) {
            filename = it->second;
            std::cout << "[UPLOAD] Filename from header: " << filename << std::endl;
        } else {
            std::cout << "[UPLOAD] No X-Filename header found, using default" << std::endl;
        }

        // Ensure shared directory exists
        std::cout << "[UPLOAD] Checking shared directory: " << g_config.sharedDir << std::endl;
        if (!FileUtils::directoryExists(g_config.sharedDir)) {
            std::cout << "[UPLOAD] Creating shared directory..." << std::endl;
            if (!FileUtils::createDirectory(g_config.sharedDir)) {
                std::cout << "[UPLOAD] Failed to create directory" << std::endl;
                res.status = 500;
                res.set_content("{\"error\": \"Failed to create shared directory\"}", "application/json");
                return;
            }
        }

        // Write file to shared directory
        std::string filepath = g_config.sharedDir + "/" + filename;
        std::cout << "[UPLOAD] Writing file to: " << filepath << std::endl;
        
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[UPLOAD] Failed to open file for writing: " << filepath << std::endl;
            res.status = 500;
            res.set_content("{\"error\": \"Failed to save file to disk\"}", "application/json");
            return;
        }

        file.write(req.body.c_str(), req.body.size());
        file.close();

        std::cout << "[UPLOAD] File saved successfully: " << filename << " (" << req.body.size() << " bytes)" << std::endl;

        // Create chunks from the uploaded file
        std::cout << "[UPLOAD] Creating chunks for file: " << filename << std::endl;
        std::vector<ChunkInfo> chunkList;
        if (FileUtils::splitFileToChunks(filepath, g_config.chunksDir, chunkList)) {
            std::cout << "[UPLOAD] Successfully created " << chunkList.size() << " chunks" << std::endl;
        } else {
            std::cout << "[UPLOAD] Failed to create chunks" << std::endl;
        }

        // If server is running, reload chunks
        if (g_server) {
            std::cout << "[UPLOAD] Reloading server chunks..." << std::endl;
            g_server->loadChunksFromDirectory();
        }

        res.set_content("{\"success\": true, \"message\": \"File uploaded and shared successfully\", \"filename\": \"" + filename + "\"}", "application/json");
    });

    http.Get("/files/shared", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (!FileUtils::directoryExists(g_config.sharedDir)) {
            res.set_content("[]", "application/json");
            return;
        }

        std::vector<std::string> files = FileUtils::listFiles(g_config.sharedDir);
        
        std::string json = "[\n";
        bool first = true;
        for (const auto& file : files) {
            if (!first) json += ",\n";
            json += "  {\"filename\": \"" + file + "\"}";
            first = false;
        }
        json += "\n]";

        res.set_content(json, "application/json");
    });

    http.Get("/files/downloads", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (!FileUtils::directoryExists(g_config.downloadDir)) {
            res.set_content("[]", "application/json");
            return;
        }

        std::vector<std::string> files = FileUtils::listFiles(g_config.downloadDir);
        
        std::string json = "[\n";
        bool first = true;
        for (const auto& file : files) {
            if (!first) json += ",\n";
            json += "  {\"filename\": \"" + file + "\"}";
            first = false;
        }
        json += "\n]";

        res.set_content(json, "application/json");
    });

    http.Post("/files/download-from-peers", [](const httplib::Request& req, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        if (req.body.empty()) {
            res.status = 400;
            res.set_content("{\"error\": \"Filename required\"}", "application/json");
            return;
        }

        if (!g_client) {
            g_client = std::make_unique<P2PClient>(
                g_config.downloadDir,
                g_config.chunksDir + "_download"
            );
            g_client->loadPeers(g_config.peersFile);
        }

        // Ensure download directory exists
        if (!FileUtils::directoryExists(g_config.downloadDir)) {
            FileUtils::createDirectory(g_config.downloadDir);
        }

        bool ok = g_client->downloadFile(req.body);
        if (!ok) {
            res.status = 500;
            res.set_content("{\"error\": \"Download failed\"}", "application/json");
            return;
        }

        res.set_content("{\"success\": true, \"message\": \"Download started\", \"filename\": \"" + req.body + "\"}", "application/json");
    });

    http.Post("/files/chunk-all", [](const httplib::Request&, httplib::Response& res) {
        std::lock_guard<std::mutex> lock(g_mutex);

        std::cout << "[CHUNK] Processing all files in shared directory..." << std::endl;

        if (!FileUtils::directoryExists(g_config.sharedDir)) {
            res.status = 400;
            res.set_content("{\"error\": \"Shared directory does not exist\"}", "application/json");
            return;
        }

        // Ensure chunks directory exists
        if (!FileUtils::directoryExists(g_config.chunksDir)) {
            FileUtils::createDirectory(g_config.chunksDir);
        }

        std::vector<std::string> files = FileUtils::listFiles(g_config.sharedDir);
        int chunkedCount = 0;
        int totalChunks = 0;

        for (const auto& file : files) {
            std::string filename = file;
            size_t lastSlash = filename.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                filename = filename.substr(lastSlash + 1);
            }

            std::cout << "[CHUNK] Processing file: " << filename << std::endl;
            
            std::vector<ChunkInfo> chunkList;
            if (FileUtils::splitFileToChunks(file, g_config.chunksDir, chunkList)) {
                std::cout << "[CHUNK] Created " << chunkList.size() << " chunks for " << filename << std::endl;
                chunkedCount++;
                totalChunks += chunkList.size();
            } else {
                std::cout << "[CHUNK] Failed to chunk file: " << filename << std::endl;
            }
        }

        // Reload server chunks
        if (g_server) {
            std::cout << "[CHUNK] Reloading server chunks..." << std::endl;
            g_server->loadChunksFromDirectory();
        }

        std::string message = "Processed " + std::to_string(chunkedCount) + " files, created " + std::to_string(totalChunks) + " chunks";
        res.set_content("{\"success\": true, \"message\": \"" + message + "\"}", "application/json");
    });

    // Serve the web interface
    http.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("web/index.html");
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            res.set_content(content, "text/html");
        } else {
            res.status = 404;
            res.set_content("Web interface not found. Make sure web/index.html exists.", "text/plain");
        }
    });

    http.listen("127.0.0.1", 8080);
}