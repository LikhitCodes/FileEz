#ifndef PEER_H
#define PEER_H

#include <string>
#include <vector>
#include <map>

// Configuration constants
const int DEFAULT_CHUNK_SIZE = 1024 * 1024; // 1MB chunks
const int MAX_BUFFER_SIZE = 8192;           // 8KB buffer
const int DEFAULT_PORT = 8080;
const int MAX_PEERS = 100;

// Chunk metadata structure
struct ChunkInfo {
    std::string filename;
    int chunkIndex;
    size_t chunkSize;
    std::string checksum;
    bool isAvailable;
    
    ChunkInfo() : chunkIndex(0), chunkSize(0), isAvailable(false) {}
    ChunkInfo(const std::string& file, int index, size_t size) 
        : filename(file), chunkIndex(index), chunkSize(size), isAvailable(true) {}
};

// Peer structure
struct Peer {
    std::string ip;
    int port;
    bool isOnline;
    std::vector<ChunkInfo> availableChunks;
    
    Peer() : port(0), isOnline(false) {}
    Peer(const std::string& ipAddr, int portNum) 
        : ip(ipAddr), port(portNum), isOnline(true) {}
    
    // Comparison operator for peer identification
    bool operator==(const Peer& other) const {
        return ip == other.ip && port == other.port;
    }
};

// File metadata structure
struct FileInfo {
    std::string filename;
    size_t totalSize;
    int totalChunks;
    std::vector<ChunkInfo> chunks;
    
    FileInfo() : totalSize(0), totalChunks(0) {}
    FileInfo(const std::string& name, size_t size, int numChunks)
        : filename(name), totalSize(size), totalChunks(numChunks) {}
};

// Function declarations for peer management
class PeerManager {
public:
    // Peer discovery and management
    bool loadPeersFromFile(const std::string& filename);
    bool savePeersToFile(const std::string& filename);
    void addPeer(const Peer& peer);
    void removePeer(const std::string& ip, int port);
    std::vector<Peer> getActivePeers();
    
    // Chunk list management
    void updatePeerChunks(const std::string& ip, int port, const std::vector<ChunkInfo>& chunks);
    std::vector<ChunkInfo> getPeerChunks(const std::string& ip, int port);
    std::vector<Peer> getPeersWithChunk(const std::string& filename, int chunkIndex);
    
    // Peer communication
    bool sendChunkList(const std::string& ip, int port, const std::vector<ChunkInfo>& chunks);
    bool requestChunkList(const std::string& ip, int port, std::vector<ChunkInfo>& chunks);
    bool pingPeer(const std::string& ip, int port);
    
private:
    std::vector<Peer> peers;
    std::map<std::string, FileInfo> fileRegistry;
};

#endif 