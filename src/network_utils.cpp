#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include "../include/network_utils.h"

// Static member initialization
bool NetworkUtils::networkInitialized = false;

bool NetworkUtils::initializeNetwork() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
#endif
    
    networkInitialized = true;
    std::cout << "Network subsystem initialized" << std::endl;
    return true;
}

void NetworkUtils::cleanupNetwork() {
#ifdef _WIN32
    if (networkInitialized) {
        WSACleanup();
    }
#endif
    networkInitialized = false;
    std::cout << "Network subsystem cleaned up" << std::endl;
}

SOCKET NetworkUtils::createServerSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create server socket: " << getLastSocketError() << std::endl;
        return INVALID_SOCKET;
    }
    
    // Set socket options
    setSocketDefaults(serverSocket);
    setSocketReuseAddr(serverSocket, true);
    
    if (!bindAndListen(serverSocket, port)) {
        closeSocket(serverSocket);
        return INVALID_SOCKET;
    }
    
    std::cout << "Server socket created and listening on port " << port << std::endl;
    return serverSocket;
}

bool NetworkUtils::bindAndListen(SOCKET serverSocket, int port) {
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to bind socket to port " << port << ": " << getLastSocketError() << std::endl;
        return false;
    }
    
    if (listen(serverSocket, LISTEN_BACKLOG) == SOCKET_ERROR) {
        std::cerr << "Failed to listen on socket: " << getLastSocketError() << std::endl;
        return false;
    }
    
    return true;
}

SOCKET NetworkUtils::acceptConnection(SOCKET serverSocket) {
    sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept connection: " << getLastSocketError() << std::endl;
        return INVALID_SOCKET;
    }
    
    setSocketDefaults(clientSocket);
    
    char* clientIP = inet_ntoa(clientAddr.sin_addr);
    std::cout << "Accepted connection from " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
    
    return clientSocket;
}

SOCKET NetworkUtils::connectToPeer(const std::string& ip, int port) {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create client socket: " << getLastSocketError() << std::endl;
        return INVALID_SOCKET;
    }
    
    setSocketDefaults(clientSocket);
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid IP address: " << ip << std::endl;
        closeSocket(clientSocket);
        return INVALID_SOCKET;
    }
    
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to " << ip << ":" << port << ": " << getLastSocketError() << std::endl;
        closeSocket(clientSocket);
        return INVALID_SOCKET;
    }
    
    std::cout << "Connected to peer " << ip << ":" << port << std::endl;
    return clientSocket;
}

bool NetworkUtils::connectWithTimeout(SOCKET clientSocket, 
                                     const std::string& ip, 
                                     int port, 
                                     int timeoutMs) {
    // TODO: Implement connection with timeout
    (void)clientSocket;
    std::cout << "Connecting to " << ip << ":" << port << " with timeout " << timeoutMs << "ms" << std::endl;
    return true;
}

bool NetworkUtils::sendData(SOCKET socket, const void* data, size_t size) {
    return sendAll(socket, data, size);
}

bool NetworkUtils::receiveData(SOCKET socket, void* buffer, size_t size) {
    return receiveAll(socket, buffer, size);
}

bool NetworkUtils::sendMessage(SOCKET socket, const MessageHeader& header, const void* data) {
    // Send header first
    if (!sendAll(socket, &header, sizeof(header))) {
        std::cerr << "Failed to send message header" << std::endl;
        return false;
    }
    
    // Send data if present
    if (header.dataSize > 0 && data != nullptr) {
        if (!sendAll(socket, data, header.dataSize)) {
            std::cerr << "Failed to send message data" << std::endl;
            return false;
        }
    }
    
    return true;
}

bool NetworkUtils::receiveMessage(SOCKET socket, MessageHeader& header, std::vector<char>& data) {
    // Receive header first
    if (!receiveAll(socket, &header, sizeof(header))) {
        std::cerr << "Failed to receive message header" << std::endl;
        return false;
    }
    
    // Receive data if present
    if (header.dataSize > 0) {
        data.resize(header.dataSize);
        if (!receiveAll(socket, data.data(), header.dataSize)) {
            std::cerr << "Failed to receive message data" << std::endl;
            return false;
        }
    } else {
        data.clear();
    }
    
    return true;
}

bool NetworkUtils::sendChunk(SOCKET socket, const std::string& chunkPath, const ChunkInfo& chunkInfo) {
    // Open chunk file
    std::ifstream chunkFile(chunkPath.c_str(), std::ios::binary);
    if (!chunkFile.is_open()) {
        std::cerr << "Failed to open chunk file: " << chunkPath << std::endl;
        return false;
    }
    
    // Get file size
    chunkFile.seekg(0, std::ios::end);
    size_t fileSize = static_cast<size_t>(chunkFile.tellg());
    chunkFile.seekg(0, std::ios::beg);
    
    // Send chunk response header
    MessageHeader header;
    header.type = MSG_CHUNK_RESPONSE;
    header.dataSize = fileSize;
    header.chunkIndex = chunkInfo.chunkIndex;
    strncpy(header.filename, chunkInfo.filename.c_str(), sizeof(header.filename) - 1);
    header.filename[sizeof(header.filename) - 1] = '\0';
    
    if (!sendMessage(socket, header)) {
        std::cerr << "Failed to send chunk header" << std::endl;
        chunkFile.close();
        return false;
    }
    
    // Send chunk data in blocks
    std::vector<char> buffer(MAX_BUFFER_SIZE);
    size_t totalSent = 0;
    
    while (totalSent < fileSize) {
        size_t toRead = std::min(static_cast<size_t>(MAX_BUFFER_SIZE), fileSize - totalSent);
        chunkFile.read(buffer.data(), static_cast<std::streamsize>(toRead));
        size_t bytesRead = static_cast<size_t>(chunkFile.gcount());
        
        if (!sendAll(socket, buffer.data(), bytesRead)) {
            std::cerr << "Failed to send chunk data" << std::endl;
            chunkFile.close();
            return false;
        }
        
        totalSent += bytesRead;
    }
    
    chunkFile.close();
    std::cout << "Sent chunk " << chunkInfo.chunkIndex << " (" << totalSent << " bytes)" << std::endl;
    return true;
}

bool NetworkUtils::receiveChunk(SOCKET socket, const std::string& outputPath, ChunkInfo& chunkInfo) {
    // Receive chunk response header
    MessageHeader header;
    std::vector<char> data;
    
    if (!receiveMessage(socket, header, data)) {
        std::cerr << "Failed to receive chunk header" << std::endl;
        return false;
    }
    
    if (header.type != MSG_CHUNK_RESPONSE) {
        std::cerr << "Invalid message type for chunk" << std::endl;
        return false;
    }
    
    // Update chunk info
    chunkInfo.chunkIndex = header.chunkIndex;
    chunkInfo.filename = header.filename;
    chunkInfo.chunkSize = header.dataSize;
    
    // Open output file
    std::ofstream outputFile(outputPath.c_str(), std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Failed to create output file: " << outputPath << std::endl;
        return false;
    }
    
    // Receive chunk data in blocks
    std::vector<char> buffer(MAX_BUFFER_SIZE);
    size_t totalReceived = 0;
    size_t remaining = header.dataSize;
    
    while (remaining > 0) {
        size_t toReceive = std::min(static_cast<size_t>(MAX_BUFFER_SIZE), remaining);
        
        if (!receiveAll(socket, buffer.data(), toReceive)) {
            std::cerr << "Failed to receive chunk data" << std::endl;
            outputFile.close();
            return false;
        }
        
        outputFile.write(buffer.data(), static_cast<std::streamsize>(toReceive));
        totalReceived += toReceive;
        remaining -= toReceive;
    }
    
    outputFile.close();
    std::cout << "Received chunk " << chunkInfo.chunkIndex << " (" << totalReceived << " bytes)" << std::endl;
    return true;
}

bool NetworkUtils::sendChunkRequest(SOCKET socket, const std::string& filename, int chunkIndex) {
    MessageHeader header;
    header.type = MSG_CHUNK_REQUEST;
    header.dataSize = 0;
    header.chunkIndex = chunkIndex;
    strncpy(header.filename, filename.c_str(), sizeof(header.filename) - 1);
    header.filename[sizeof(header.filename) - 1] = '\0';
    
    return sendMessage(socket, header);
}

bool NetworkUtils::sendChunkList(SOCKET socket, const std::vector<ChunkInfo>& chunks) {
    // Send chunk list header
    MessageHeader header;
    header.type = MSG_CHUNK_LIST_RESPONSE;
    header.dataSize = 0; // We'll send chunks separately
    header.chunkIndex = static_cast<int>(chunks.size());
    
    if (!sendAll(socket, &header, sizeof(header))) {
        std::cerr << "Failed to send chunk list header" << std::endl;
        return false;
    }
    
    // Send each chunk info
    for (const auto& chunk : chunks) {
        if (!sendAll(socket, &chunk, sizeof(ChunkInfo))) {
            std::cerr << "Failed to send chunk info" << std::endl;
            return false;
        }
    }
    
    std::cout << "Sent chunk list with " << chunks.size() << " chunks" << std::endl;
    return true;
}

bool NetworkUtils::receiveChunkList(SOCKET socket, std::vector<ChunkInfo>& chunks) {
    // Receive chunk list header
    MessageHeader header;
    
    if (!receiveAll(socket, &header, sizeof(header))) {
        std::cerr << "Failed to receive chunk list header" << std::endl;
        return false;
    }
    
    if (header.type != MSG_CHUNK_LIST_RESPONSE) {
        std::cerr << "Invalid message type for chunk list: " << header.type << std::endl;
        return false;
    }
    
    int chunkCount = header.chunkIndex;
    chunks.clear();
    chunks.reserve(chunkCount);
    
    // Receive each chunk info
    for (int i = 0; i < chunkCount; i++) {
        ChunkInfo chunk;
        if (!receiveAll(socket, &chunk, sizeof(ChunkInfo))) {
            std::cerr << "Failed to receive chunk info " << i << std::endl;
            return false;
        }
        chunks.push_back(chunk);
    }
    
    std::cout << "Received chunk list with " << chunks.size() << " chunks" << std::endl;
    return true;
}

bool NetworkUtils::setSocketTimeout(SOCKET socket, int timeoutMs) {
#ifdef _WIN32
    DWORD timeout = timeoutMs;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR ||
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
        return false;
    }
#else
    struct timeval timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == SOCKET_ERROR ||
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == SOCKET_ERROR) {
        return false;
    }
#endif
    return true;
}

bool NetworkUtils::setSocketNonBlocking(SOCKET socket, bool nonBlocking) {
#ifdef _WIN32
    u_long mode = nonBlocking ? 1 : 0;
    return ioctlsocket(socket, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return false;
    
    if (nonBlocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    
    return fcntl(socket, F_SETFL, flags) == 0;
#endif
}

bool NetworkUtils::setSocketReuseAddr(SOCKET socket, bool reuse) {
    int optval = reuse ? 1 : 0;
    return setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) != SOCKET_ERROR;
}

std::string NetworkUtils::getLocalIP() {
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        return "127.0.0.1";
    }
    
    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr || host->h_addr_list[0] == nullptr) {
        return "127.0.0.1";
    }
    
    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
    return std::string(inet_ntoa(addr));
#else
    // For Linux/Unix systems
    return "127.0.0.1";
#endif
}

bool NetworkUtils::isValidIP(const std::string& ip) {
    return inet_addr(ip.c_str()) != INADDR_NONE;
}

bool NetworkUtils::isPortAvailable(int port) {
    SOCKET testSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (testSocket == INVALID_SOCKET) {
        return false;
    }
    
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    bool available = (bind(testSocket, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR);
    closeSocket(testSocket);
    
    return available;
}

std::string NetworkUtils::getLastSocketError() {
#ifdef _WIN32
    int error = WSAGetLastError();
    return "Error code: " + std::to_string(error);
#else
    return strerror(errno);
#endif
}

void NetworkUtils::closeSocket(SOCKET socket) {
    if (socket != INVALID_SOCKET) {
        closesocket(socket);
    }
}

std::string NetworkUtils::socketToString(SOCKET socket) {
    return "Socket(" + std::to_string(socket) + ")";
}

bool NetworkUtils::isSocketConnected(SOCKET socket) {
    if (socket == INVALID_SOCKET) {
        return false;
    }
    
    // Try to peek at the socket to see if it's still connected
    char buffer[1];
    int result = recv(socket, buffer, 1, MSG_PEEK);
    
    if (result == SOCKET_ERROR) {
#ifdef _WIN32
        int error = WSAGetLastError();
        return (error == WSAEWOULDBLOCK || error == WSAEINPROGRESS);
#else
        return (errno == EWOULDBLOCK || errno == EINPROGRESS);
#endif
    }
    
    return result >= 0;
}

bool NetworkUtils::sendAll(SOCKET socket, const void* data, size_t size) {
    const char* buffer = static_cast<const char*>(data);
    size_t totalSent = 0;
    
    while (totalSent < size) {
        int sent = send(socket, buffer + totalSent, static_cast<int>(size - totalSent), 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "Send failed: " << getLastSocketError() << std::endl;
            return false;
        }
        totalSent += sent;
    }
    
    return true;
}

bool NetworkUtils::receiveAll(SOCKET socket, void* buffer, size_t size) {
    char* buf = static_cast<char*>(buffer);
    size_t totalReceived = 0;
    
    while (totalReceived < size) {
        int received = recv(socket, buf + totalReceived, static_cast<int>(size - totalReceived), 0);
        if (received == SOCKET_ERROR) {
            std::cerr << "Receive failed: " << getLastSocketError() << std::endl;
            return false;
        }
        if (received == 0) {
            std::cerr << "Connection closed by peer" << std::endl;
            return false;
        }
        totalReceived += received;
    }
    
    return true;
}

void NetworkUtils::setSocketDefaults(SOCKET socket) {
    setSocketTimeout(socket, RECV_TIMEOUT);
}