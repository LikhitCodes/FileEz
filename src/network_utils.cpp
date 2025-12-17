#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <algorithm>
#ifdef _WIN32
    #include <ws2tcpip.h>
#else
    #include <ifaddrs.h>
#endif
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
    // Receive chunk response header and data
    MessageHeader header;
    std::vector<char> data;
    
    if (!receiveMessage(socket, header, data)) {
        std::cerr << "Failed to receive chunk message" << std::endl;
        return false;
    }
    
    if (header.type != MSG_CHUNK_RESPONSE) {
        std::cerr << "Invalid message type for chunk (expected " << MSG_CHUNK_RESPONSE 
                  << ", got " << header.type << ")" << std::endl;
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
    
    // Write the data that was already received by receiveMessage
    if (!data.empty()) {
        outputFile.write(data.data(), static_cast<std::streamsize>(data.size()));
    }
    
    outputFile.close();
    std::cout << "Received chunk " << chunkInfo.chunkIndex << " (" << data.size() << " bytes)" << std::endl;
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
    
    // Send each chunk info (properly serialized)
    for (const auto& chunk : chunks) {
        // Send filename length
        uint32_t filenameLen = static_cast<uint32_t>(chunk.filename.length());
        if (!sendAll(socket, &filenameLen, sizeof(filenameLen))) {
            std::cerr << "Failed to send filename length" << std::endl;
            return false;
        }
        
        // Send filename
        if (filenameLen > 0) {
            if (!sendAll(socket, chunk.filename.c_str(), filenameLen)) {
                std::cerr << "Failed to send filename" << std::endl;
                return false;
            }
        }
        
        // Send chunk index
        if (!sendAll(socket, &chunk.chunkIndex, sizeof(chunk.chunkIndex))) {
            std::cerr << "Failed to send chunk index" << std::endl;
            return false;
        }
        
        // Send chunk size
        if (!sendAll(socket, &chunk.chunkSize, sizeof(chunk.chunkSize))) {
            std::cerr << "Failed to send chunk size" << std::endl;
            return false;
        }
        
        // Send checksum length
        uint32_t checksumLen = static_cast<uint32_t>(chunk.checksum.length());
        if (!sendAll(socket, &checksumLen, sizeof(checksumLen))) {
            std::cerr << "Failed to send checksum length" << std::endl;
            return false;
        }
        
        // Send checksum
        if (checksumLen > 0) {
            if (!sendAll(socket, chunk.checksum.c_str(), checksumLen)) {
                std::cerr << "Failed to send checksum" << std::endl;
                return false;
            }
        }
        
        // Send isAvailable flag
        if (!sendAll(socket, &chunk.isAvailable, sizeof(chunk.isAvailable))) {
            std::cerr << "Failed to send isAvailable flag" << std::endl;
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
    
    // Receive each chunk info (properly deserialized)
    for (int i = 0; i < chunkCount; i++) {
        ChunkInfo chunk;
        
        // Receive filename length
        uint32_t filenameLen = 0;
        if (!receiveAll(socket, &filenameLen, sizeof(filenameLen))) {
            std::cerr << "Failed to receive filename length" << std::endl;
            return false;
        }
        
        // Receive filename
        if (filenameLen > 0) {
            std::vector<char> filenameBuf(filenameLen);
            if (!receiveAll(socket, filenameBuf.data(), filenameLen)) {
                std::cerr << "Failed to receive filename" << std::endl;
                return false;
            }
            chunk.filename = std::string(filenameBuf.begin(), filenameBuf.end());
        }
        
        // Receive chunk index
        if (!receiveAll(socket, &chunk.chunkIndex, sizeof(chunk.chunkIndex))) {
            std::cerr << "Failed to receive chunk index" << std::endl;
            return false;
        }
        
        // Receive chunk size
        if (!receiveAll(socket, &chunk.chunkSize, sizeof(chunk.chunkSize))) {
            std::cerr << "Failed to receive chunk size" << std::endl;
            return false;
        }
        
        // Receive checksum length
        uint32_t checksumLen = 0;
        if (!receiveAll(socket, &checksumLen, sizeof(checksumLen))) {
            std::cerr << "Failed to receive checksum length" << std::endl;
            return false;
        }
        
        // Receive checksum
        if (checksumLen > 0) {
            std::vector<char> checksumBuf(checksumLen);
            if (!receiveAll(socket, checksumBuf.data(), checksumLen)) {
                std::cerr << "Failed to receive checksum" << std::endl;
                return false;
            }
            chunk.checksum = std::string(checksumBuf.begin(), checksumBuf.end());
        }
        
        // Receive isAvailable flag
        if (!receiveAll(socket, &chunk.isAvailable, sizeof(chunk.isAvailable))) {
            std::cerr << "Failed to receive isAvailable flag" << std::endl;
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

// Network discovery functions
std::vector<std::string> NetworkUtils::getAllLocalIPs() {
    std::vector<std::string> ips;
    
#ifdef _WIN32
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        std::cerr << "Failed to get hostname" << std::endl;
        return ips;
    }
    
    struct hostent* host = gethostbyname(hostname);
    if (host == nullptr) {
        std::cerr << "Failed to get host by name" << std::endl;
        return ips;
    }
    
    for (int i = 0; host->h_addr_list[i] != nullptr; i++) {
        struct in_addr addr;
        memcpy(&addr, host->h_addr_list[i], sizeof(struct in_addr));
        std::string ip = inet_ntoa(addr);
        if (ip != "127.0.0.1") {
            ips.push_back(ip);
        }
    }
#else
    // Linux implementation
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        return ips;
    }
    
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN);
            std::string ip(ipStr);
            if (ip != "127.0.0.1") {
                ips.push_back(ip);
            }
        }
    }
    freeifaddrs(ifaddr);
#endif
    
    return ips;
}

std::string NetworkUtils::getSubnetMask() {
    // For simplicity, assume /24 subnet (255.255.255.0)
    return "255.255.255.0";
}

bool NetworkUtils::isPeerResponding(const std::string& ip, int port, int timeoutMs) {
    SOCKET socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket == INVALID_SOCKET) {
        return false;
    }
    
    // Set socket to non-blocking mode for timeout
    setSocketNonBlocking(socket, true);
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    // Try to connect
    connect(socket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    
    // Wait for connection with timeout
    fd_set writeSet;
    FD_ZERO(&writeSet);
    FD_SET(socket, &writeSet);
    
    struct timeval timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;
    
    int result = select(static_cast<int>(socket) + 1, nullptr, &writeSet, nullptr, &timeout);
    
    bool responding = false;
    if (result > 0) {
        // Check if connection succeeded
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == 0 && error == 0) {
            // Connection successful, try to send a ping
            setSocketNonBlocking(socket, false);
            
            MessageHeader pingHeader;
            pingHeader.type = MSG_PEER_PING;
            pingHeader.dataSize = 0;
            
            if (sendAll(socket, &pingHeader, sizeof(pingHeader))) {
                // Try to receive pong
                MessageHeader pongHeader;
                setSocketTimeout(socket, timeoutMs);
                if (receiveAll(socket, &pongHeader, sizeof(pongHeader))) {
                    responding = (pongHeader.type == MSG_PEER_PONG);
                }
            }
        }
    }
    
    closeSocket(socket);
    return responding;
}

std::vector<std::string> NetworkUtils::discoverPeersOnNetwork(int port, int timeoutMs) {
    std::vector<std::string> discoveredPeers;
    
    std::cout << "\n=== Discovering peers on local network ===" << std::endl;
    std::cout << "Scanning for P2P servers on port " << port << "..." << std::endl;
    
    // Get local IPs
    std::vector<std::string> localIPs = getAllLocalIPs();
    
    if (localIPs.empty()) {
        std::cout << "No local network interfaces found" << std::endl;
        return discoveredPeers;
    }
    
    std::cout << "Local IP addresses:" << std::endl;
    for (const auto& ip : localIPs) {
        std::cout << "  - " << ip << std::endl;
    }
    
    // For each local IP, scan the subnet
    for (const auto& localIP : localIPs) {
        // Extract subnet (assume /24)
        size_t lastDot = localIP.find_last_of('.');
        if (lastDot == std::string::npos) continue;
        
        std::string subnet = localIP.substr(0, lastDot + 1);
        std::cout << "\nScanning subnet: " << subnet << "0/24" << std::endl;
        
        // Scan common IP range (1-254)
        for (int i = 1; i <= 254; i++) {
            std::string testIP = subnet + std::to_string(i);
            
            // Skip our own IP
            bool isOwnIP = false;
            for (const auto& lip : localIPs) {
                if (testIP == lip) {
                    isOwnIP = true;
                    break;
                }
            }
            if (isOwnIP) continue;
            
            // Show progress every 50 IPs
            if (i % 50 == 0) {
                std::cout << "  Scanned " << i << "/254 addresses..." << std::endl;
            }
            
            // Quick check if peer is responding
            if (isPeerResponding(testIP, port, timeoutMs)) {
                std::cout << "  ✓ Found peer: " << testIP << ":" << port << std::endl;
                discoveredPeers.push_back(testIP);
            }
        }
    }
    
    std::cout << "\nDiscovery complete. Found " << discoveredPeers.size() << " peers." << std::endl;
    return discoveredPeers;
}
