#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include "peer.h"
#include "network_utils.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

class P2PServer {
private:
    int port;
    SOCKET serverSocket;
    bool running;
    std::vector<ChunkInfo> availableChunks;
    PeerManager peerManager;
    std::string chunksDirectory;
    
#ifdef _WIN32
    HANDLE serverThread;
    std::vector<HANDLE> clientThreads;
#else
    pthread_t serverThread;
    std::vector<pthread_t> clientThreads;
#endif

public:
    P2PServer(int serverPort, const std::string& chunksDir = "data/chunks");
    ~P2PServer();
    
    bool start();
    void stop();
    void run();
    void handleClient(SOCKET clientSocket);
    void addAvailableChunk(const ChunkInfo& chunk);
    void removeAvailableChunk(const std::string& filename, int chunkIndex);
    std::vector<ChunkInfo> getAvailableChunks();
    void loadChunksFromDirectory();
    bool isRunning() const { return running; }
    
private:
    void processChunkRequest(SOCKET clientSocket, const MessageHeader& header);
    void processChunkListRequest(SOCKET clientSocket);
    void processPingRequest(SOCKET clientSocket);
    void sendErrorResponse(SOCKET clientSocket, const std::string& error);
    
#ifdef _WIN32
    static unsigned __stdcall serverThreadFunc(void* param);
    static unsigned __stdcall clientThreadFunc(void* param);
#else
    static void* serverThreadFunc(void* param);
    static void* clientThreadFunc(void* param);
#endif
};

#endif // SERVER_H
