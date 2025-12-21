#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <string>
#include <vector>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#include "peer.h"

// Network configuration constants
const int LISTEN_BACKLOG = 10;
const int CONNECT_TIMEOUT = 5000; // 5 seconds
const int RECV_TIMEOUT = 10000;   // 10 seconds
const int SEND_TIMEOUT = 10000;   // 10 seconds

// Message types for peer communication
// Protocol:
// 1. Client sends MSG_CHUNK_LIST_REQUEST -> Server responds with MSG_CHUNK_LIST_RESPONSE
// 2. Client sends MSG_CHUNK_REQUEST -> Server responds with MSG_CHUNK_RESPONSE
// 3. Client sends MSG_PEER_PING -> Server responds with MSG_PEER_PONG
enum MessageType {
    MSG_CHUNK_REQUEST = 1,       // Request a specific chunk from peer
    MSG_CHUNK_RESPONSE = 2,      // Response containing chunk data
    MSG_CHUNK_LIST_REQUEST = 3,  // Request list of available chunks
    MSG_CHUNK_LIST_RESPONSE = 4, // Response with chunk list
    MSG_PEER_PING = 5,           // Ping to check peer availability
    MSG_PEER_PONG = 6,           // Pong response
    MSG_ERROR = 7                // Error message
};

// Message header structure
struct MessageHeader {
    MessageType type;
    size_t dataSize;
    char filename[256];
    int chunkIndex;
    
    MessageHeader() : type(MSG_ERROR), dataSize(0), chunkIndex(-1) {
        memset(filename, 0, sizeof(filename));
    }
};

// Network utilities class
class NetworkUtils {
public:
    // Socket initialization and cleanup
    static bool initializeNetwork();
    static void cleanupNetwork();
    
    // Server socket operations
    static SOCKET createServerSocket(int port);
    static bool bindAndListen(SOCKET serverSocket, int port);
    static SOCKET acceptConnection(SOCKET serverSocket);
    
    // Client socket operations
    static SOCKET connectToPeer(const std::string& ip, int port);
    static bool connectWithTimeout(SOCKET clientSocket, 
                                  const std::string& ip, 
                                  int port, 
                                  int timeoutMs);
    
    // Data transmission
    static bool sendData(SOCKET socket, const void* data, size_t size);
    static bool receiveData(SOCKET socket, void* buffer, size_t size);
    static bool sendMessage(SOCKET socket, const MessageHeader& header, const void* data = nullptr);
    static bool receiveMessage(SOCKET socket, MessageHeader& header, std::vector<char>& data);
    
    // Chunk transfer operations
    static bool sendChunk(SOCKET socket, const std::string& chunkPath, const ChunkInfo& chunkInfo);
    static bool receiveChunk(SOCKET socket, const std::string& outputPath, ChunkInfo& chunkInfo);
    
    // Peer communication
    static bool sendChunkRequest(SOCKET socket, const std::string& filename, int chunkIndex);
    static bool sendChunkList(SOCKET socket, const std::vector<ChunkInfo>& chunks);
    static bool receiveChunkList(SOCKET socket, std::vector<ChunkInfo>& chunks);
    
    // Socket configuration
    static bool setSocketTimeout(SOCKET socket, int timeoutMs);
    static bool setSocketNonBlocking(SOCKET socket, bool nonBlocking);
    static bool setSocketReuseAddr(SOCKET socket, bool reuse);
    
    // Network information
    static std::string getLocalIP();
    static std::vector<std::string> getAllLocalIPs();
    static std::string getSubnetMask();
    static std::string getClientIP(SOCKET socket);
    static bool isValidIP(const std::string& ip);
    static bool isPortAvailable(int port);
    
    // Peer discovery
    static std::vector<std::string> discoverPeersOnNetwork(int port, int timeoutMs = 1000);
    static bool isPeerResponding(const std::string& ip, int port, int timeoutMs = 1000);
    
    // Error handling
    static std::string getLastSocketError();
    static void closeSocket(SOCKET socket);
    
    // Utility functions
    static std::string socketToString(SOCKET socket);
    static bool isSocketConnected(SOCKET socket);

private:
    static bool networkInitialized;
    
    // Helper functions
    static bool sendAll(SOCKET socket, const void* data, size_t size);
    static bool receiveAll(SOCKET socket, void* buffer, size_t size);
    static void setSocketDefaults(SOCKET socket);
};

#endif // NETWORK_UTILS_H