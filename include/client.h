#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <vector>
#include "peer.h"
#include "network_utils.h"

class P2PClient {
private:
    PeerManager peerManager;
    std::string downloadDir;
    std::string chunksDir;
    
public:
    P2PClient(const std::string& downloadDirectory, const std::string& chunksDirectory);
    
    bool downloadFile(const std::string& filename);
    bool downloadFileFromPeers(const std::string& filename, const std::vector<std::string>& peerAddresses);
    bool requestChunkFromPeer(const std::string& ip, int port, const std::string& filename, int chunkIndex);
    std::vector<ChunkInfo> requestChunkListFromPeer(const std::string& ip, int port);
    bool pingPeer(const std::string& ip, int port);
    bool loadPeers(const std::string& peersFile);
    
private:
    std::vector<Peer> findPeersWithFile(const std::string& filename);
    bool downloadChunk(const Peer& peer, const std::string& filename, int chunkIndex);
    void assignChunksToPeers(const std::string& filename, int totalChunks, 
                           const std::vector<Peer>& peers,
                           std::vector<std::pair<Peer, int>>& assignments);
    bool mergeDownloadedChunks(const std::string& filename, int totalChunks);
    void updateDownloadProgress(const std::string& filename, int completedChunks, int totalChunks);
    
    struct ChunkDownloadTask;
    static void downloadChunkTask(ChunkDownloadTask* task);
    bool downloadChunksParallel(const std::vector<ChunkDownloadTask>& tasks);
};

#endif // CLIENT_H
