#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif
#include "../include/server.h"
#include "../include/peer.h"
#include "../include/file_utils.h"
#include "../include/network_utils.h"

// Structure to pass data to client handler thread
struct ClientHandlerData {
    SOCKET clientSocket;
    P2PServer* server;
    
    ClientHandlerData(SOCKET socket, P2PServer* srv) 
        : clientSocket(socket), server(srv) {}
};

// Constructor
P2PServer::P2PServer(int serverPort, const std::string& chunksDir) 
    : port(serverPort), serverSocket(INVALID_SOCKET), running(false), 
      chunksDirectory(chunksDir) {
#ifdef _WIN32
    serverThread = NULL;
#else
    serverThread = 0;
#endif
}

// Destructor
P2PServer::~P2PServer() {
    stop();
}

bool P2PServer::start() {
    if (running) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }
    
    std::cout << "Starting P2P server on port " << port << std::endl;
    
    // Load available chunks from directory
    loadChunksFromDirectory();
    
    // Create server socket
    serverSocket = NetworkUtils::createServerSocket(port);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket" << std::endl;
        return false;
    }
    
    running = true;
    
    // Start server thread
#ifdef _WIN32
    serverThread = (HANDLE)_beginthreadex(NULL, 0, serverThreadFunc, this, 0, NULL);
    if (serverThread == NULL) {
        std::cerr << "Failed to create server thread" << std::endl;
        running = false;
        NetworkUtils::closeSocket(serverSocket);
        return false;
    }
#else
    if (pthread_create(&serverThread, NULL, serverThreadFunc, this) != 0) {
        std::cerr << "Failed to create server thread" << std::endl;
        running = false;
        NetworkUtils::closeSocket(serverSocket);
        return false;
    }
#endif
    
    std::cout << "P2P server started successfully on port " << port << std::endl;
    std::cout << "Serving " << availableChunks.size() << " chunks" << std::endl;
    return true;
}

void P2PServer::stop() {
    if (!running) {
        return;
    }
    
    std::cout << "Stopping P2P server..." << std::endl;
    running = false;
    
    // Close server socket to unblock accept()
    if (serverSocket != INVALID_SOCKET) {
        NetworkUtils::closeSocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    
    // Wait for server thread to finish
#ifdef _WIN32
    if (serverThread != NULL) {
        WaitForSingleObject(serverThread, 5000); // Wait up to 5 seconds
        CloseHandle(serverThread);
        serverThread = NULL;
    }
    
    // Wait for client threads
    for (HANDLE thread : clientThreads) {
        WaitForSingleObject(thread, 2000);
        CloseHandle(thread);
    }
#else
    if (serverThread != 0) {
        pthread_join(serverThread, NULL);
        serverThread = 0;
    }
    
    // Wait for client threads
    for (pthread_t thread : clientThreads) {
        pthread_join(thread, NULL);
    }
#endif
    
    clientThreads.clear();
    std::cout << "P2P server stopped" << std::endl;
}

void P2PServer::run() {
    std::cout << "Server listening for connections..." << std::endl;
    
    while (running) {
        SOCKET clientSocket = NetworkUtils::acceptConnection(serverSocket);
        
        if (clientSocket == INVALID_SOCKET) {
            if (running) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }
        
        // Create client handler data
        ClientHandlerData* data = new ClientHandlerData(clientSocket, this);
        
        // Start client handler thread
#ifdef _WIN32
        HANDLE clientThread = (HANDLE)_beginthreadex(NULL, 0, clientThreadFunc, data, 0, NULL);
        if (clientThread != NULL) {
            clientThreads.push_back(clientThread);
        } else {
            std::cerr << "Failed to create client handler thread" << std::endl;
            NetworkUtils::closeSocket(clientSocket);
            delete data;
        }
#else
        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, clientThreadFunc, data) == 0) {
            pthread_detach(clientThread); // Detach so it cleans up automatically
            clientThreads.push_back(clientThread);
        } else {
            std::cerr << "Failed to create client handler thread" << std::endl;
            NetworkUtils::closeSocket(clientSocket);
            delete data;
        }
#endif
    }
}

void P2PServer::handleClient(SOCKET clientSocket) {
    std::cout << "Handling client request..." << std::endl;
    
    // Receive message header
    MessageHeader header;
    std::vector<char> data;
    
    if (!NetworkUtils::receiveMessage(clientSocket, header, data)) {
        std::cerr << "Failed to receive message from client" << std::endl;
        NetworkUtils::closeSocket(clientSocket);
        return;
    }
    
    // Process based on message type
    switch (header.type) {
        case MSG_CHUNK_REQUEST:
            processChunkRequest(clientSocket, header);
            break;
            
        case MSG_CHUNK_LIST_REQUEST:
            processChunkListRequest(clientSocket);
            break;
            
        case MSG_PEER_PING:
            processPingRequest(clientSocket);
            break;
            
        default:
            std::cerr << "Unknown message type: " << header.type << std::endl;
            sendErrorResponse(clientSocket, "Unknown message type");
            break;
    }
    
    NetworkUtils::closeSocket(clientSocket);
}

void P2PServer::addAvailableChunk(const ChunkInfo& chunk) {
    availableChunks.push_back(chunk);
}

void P2PServer::removeAvailableChunk(const std::string& filename, int chunkIndex) {
    auto it = std::remove_if(availableChunks.begin(), availableChunks.end(),
        [&filename, chunkIndex](const ChunkInfo& chunk) {
            return chunk.filename == filename && chunk.chunkIndex == chunkIndex;
        });
    
    if (it != availableChunks.end()) {
        availableChunks.erase(it, availableChunks.end());
        std::cout << "Removed chunk: " << filename << " [" << chunkIndex << "]" << std::endl;
    }
}

std::vector<ChunkInfo> P2PServer::getAvailableChunks() {
    return availableChunks;
}

void P2PServer::loadChunksFromDirectory() {
    if (!FileUtils::directoryExists(chunksDirectory)) {
        std::cout << "Chunks directory does not exist: " << chunksDirectory << std::endl;
        return;
    }
    
    std::vector<std::string> files = FileUtils::listFiles(chunksDirectory);
    availableChunks.clear();
    
    for (const auto& file : files) {
        std::string filename = file;
        size_t lastSlash = filename.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filename.substr(lastSlash + 1);
        }
        
        // Check if it's a chunk file (contains ".chunk.")
        if (filename.find(".chunk.") != std::string::npos) {
            // Parse chunk filename: originalname.chunk.0001
            size_t chunkPos = filename.find(".chunk.");
            if (chunkPos != std::string::npos) {
                std::string originalName = filename.substr(0, chunkPos);
                std::string indexStr = filename.substr(chunkPos + 7); // Skip ".chunk."
                
                try {
                    int chunkIndex = std::stoi(indexStr);
                    size_t chunkSize = FileUtils::getFileSize(file);
                    
                    ChunkInfo chunk(originalName, chunkIndex, chunkSize);
                    chunk.checksum = FileUtils::computeChunkHash(file);
                    chunk.isAvailable = true;
                    
                    availableChunks.push_back(chunk);
                } catch (...) {
                    std::cerr << "Failed to parse chunk index from: " << filename << std::endl;
                }
            }
        }
    }
    
    std::cout << "Loaded " << availableChunks.size() << " chunks from " << chunksDirectory << std::endl;
}

void P2PServer::processChunkRequest(SOCKET clientSocket, const MessageHeader& header) {
    std::cout << "Processing chunk request: " << header.filename << " [" << header.chunkIndex << "]" << std::endl;
    
    // Find the requested chunk
    ChunkInfo* requestedChunk = nullptr;
    for (auto& chunk : availableChunks) {
        if (chunk.filename == header.filename && chunk.chunkIndex == header.chunkIndex) {
            requestedChunk = &chunk;
            break;
        }
    }
    
    if (requestedChunk == nullptr) {
        std::cerr << "Chunk not found: " << header.filename << " [" << header.chunkIndex << "]" << std::endl;
        sendErrorResponse(clientSocket, "Chunk not found");
        return;
    }
    
    // Get chunk file path
    std::string chunkPath = FileUtils::getChunkPath(chunksDirectory, 
                                                    requestedChunk->filename, 
                                                    requestedChunk->chunkIndex);
    
    if (!FileUtils::fileExists(chunkPath)) {
        std::cerr << "Chunk file does not exist: " << chunkPath << std::endl;
        sendErrorResponse(clientSocket, "Chunk file not found");
        return;
    }
    
    // Send the chunk
    if (NetworkUtils::sendChunk(clientSocket, chunkPath, *requestedChunk)) {
        std::cout << "Successfully sent chunk: " << header.filename << " [" << header.chunkIndex << "]" << std::endl;
    } else {
        std::cerr << "Failed to send chunk: " << header.filename << " [" << header.chunkIndex << "]" << std::endl;
    }
}

void P2PServer::processChunkListRequest(SOCKET clientSocket) {
    std::cout << "Processing chunk list request" << std::endl;
    
    if (NetworkUtils::sendChunkList(clientSocket, availableChunks)) {
        std::cout << "Successfully sent chunk list (" << availableChunks.size() << " chunks)" << std::endl;
    } else {
        std::cerr << "Failed to send chunk list" << std::endl;
    }
}

void P2PServer::processPingRequest(SOCKET clientSocket) {
    std::cout << "Processing ping request" << std::endl;
    
    // Send pong response
    MessageHeader pongHeader;
    pongHeader.type = MSG_PEER_PONG;
    pongHeader.dataSize = 0;
    
    if (NetworkUtils::sendMessage(clientSocket, pongHeader)) {
        std::cout << "Sent pong response" << std::endl;
    } else {
        std::cerr << "Failed to send pong response" << std::endl;
    }
}

void P2PServer::sendErrorResponse(SOCKET clientSocket, const std::string& error) {
    MessageHeader errorHeader;
    errorHeader.type = MSG_ERROR;
    errorHeader.dataSize = error.length();
    
    NetworkUtils::sendMessage(clientSocket, errorHeader, error.c_str());
    std::cerr << "Sent error response: " << error << std::endl;
}

// Thread functions
#ifdef _WIN32
unsigned __stdcall P2PServer::serverThreadFunc(void* param) {
    P2PServer* server = static_cast<P2PServer*>(param);
    server->run();
    return 0;
}

unsigned __stdcall P2PServer::clientThreadFunc(void* param) {
    ClientHandlerData* data = static_cast<ClientHandlerData*>(param);
    data->server->handleClient(data->clientSocket);
    delete data;
    return 0;
}
#else
void* P2PServer::serverThreadFunc(void* param) {
    P2PServer* server = static_cast<P2PServer*>(param);
    server->run();
    return NULL;
}

void* P2PServer::clientThreadFunc(void* param) {
    ClientHandlerData* data = static_cast<ClientHandlerData*>(param);
    data->server->handleClient(data->clientSocket);
    delete data;
    return NULL;
}
#endif