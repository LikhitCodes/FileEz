#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif
#include "../include/client.h"
#include "../include/peer.h"
#include "../include/file_utils.h"
#include "../include/network_utils.h"

// Structure for chunk download task
struct P2PClient::ChunkDownloadTask {
    std::string ip;
    int port;
    std::string filename;
    int chunkIndex;
    std::string outputPath;
    bool success;
    
    ChunkDownloadTask() : port(0), chunkIndex(0), success(false) {}
    ChunkDownloadTask(const std::string& peerIp, int peerPort, 
                     const std::string& file, int index, const std::string& output)
        : ip(peerIp), port(peerPort), filename(file), 
          chunkIndex(index), outputPath(output), success(false) {}
};

// Constructor
P2PClient::P2PClient(const std::string& downloadDirectory, const std::string& chunksDirectory)
    : downloadDir(downloadDirectory), chunksDir(chunksDirectory) {
    // Ensure directories exist
    FileUtils::createDirectory(downloadDir);
    FileUtils::createDirectory(chunksDir);
}

bool P2PClient::loadPeers(const std::string& peersFile) {
    return peerManager.loadPeersFromFile(peersFile);
}

bool P2PClient::downloadFile(const std::string& filename) {
    std::cout << "\n=== Starting download for file: " << filename << " ===" << std::endl;
    
    // Step 1: Find peers that have the file
    std::vector<Peer> peersWithFile = findPeersWithFile(filename);
    
    if (peersWithFile.empty()) {
        std::cerr << "No peers found with file: " << filename << std::endl;
        return false;
    }
    
    std::cout << "Found " << peersWithFile.size() << " peers with the file" << std::endl;
    
    // Step 2: Get chunk information from first available peer
    std::vector<ChunkInfo> allChunks;
    for (const auto& peer : peersWithFile) {
        allChunks = requestChunkListFromPeer(peer.ip, peer.port);
        if (!allChunks.empty()) {
            break;
        }
    }
    
    if (allChunks.empty()) {
        std::cerr << "Could not get chunk information from any peer" << std::endl;
        return false;
    }
    
    // Filter chunks for this specific file
    std::vector<ChunkInfo> fileChunks;
    for (const auto& chunk : allChunks) {
        if (chunk.filename == filename) {
            fileChunks.push_back(chunk);
        }
    }
    
    if (fileChunks.empty()) {
        std::cerr << "No chunks found for file: " << filename << std::endl;
        return false;
    }
    
    std::cout << "File has " << fileChunks.size() << " chunks" << std::endl;
    
    // Step 3: Assign chunks to different peers (round-robin)
    std::vector<std::pair<Peer, int>> assignments;
    assignChunksToPeers(filename, static_cast<int>(fileChunks.size()), peersWithFile, assignments);
    
    // Step 4: Download chunks in parallel
    std::vector<ChunkDownloadTask> tasks;
    for (const auto& assignment : assignments) {
        std::string chunkPath = FileUtils::getChunkPath(chunksDir, filename, assignment.second);
        ChunkDownloadTask task(assignment.first.ip, assignment.first.port, 
                              filename, assignment.second, chunkPath);
        tasks.push_back(task);
    }
    
    std::cout << "Downloading " << tasks.size() << " chunks..." << std::endl;
    
    if (!downloadChunksParallel(tasks)) {
        std::cerr << "Failed to download all chunks" << std::endl;
        return false;
    }
    
    // Step 5: Merge chunks into final file
    std::string outputFile = downloadDir + "/" + filename;
    if (!mergeDownloadedChunks(filename, static_cast<int>(fileChunks.size()))) {
        std::cerr << "Failed to merge chunks" << std::endl;
        return false;
    }
    
    std::cout << "=== Download complete: " << outputFile << " ===" << std::endl;
    return true;
}

bool P2PClient::downloadFileFromPeers(const std::string& filename, 
                                     const std::vector<std::string>& peerAddresses) {
    std::cout << "\n=== Downloading " << filename << " from specified peers ===" << std::endl;
    
    // Parse peer addresses and add to peer manager
    for (const auto& address : peerAddresses) {
        size_t colonPos = address.find(':');
        if (colonPos != std::string::npos) {
            std::string ip = address.substr(0, colonPos);
            int port = std::stoi(address.substr(colonPos + 1));
            Peer peer(ip, port);
            peerManager.addPeer(peer);
        }
    }
    
    return downloadFile(filename);
}

bool P2PClient::requestChunkFromPeer(const std::string& ip, int port, 
                                    const std::string& filename, int chunkIndex) {
    std::cout << "Requesting chunk " << chunkIndex << " of " << filename 
              << " from peer " << ip << ":" << port << std::endl;
    
    // Connect to peer
    SOCKET socket = NetworkUtils::connectToPeer(ip, port);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Failed to connect to peer " << ip << ":" << port << std::endl;
        return false;
    }
    
    // Send chunk request
    if (!NetworkUtils::sendChunkRequest(socket, filename, chunkIndex)) {
        std::cerr << "Failed to send chunk request" << std::endl;
        NetworkUtils::closeSocket(socket);
        return false;
    }
    
    // Receive chunk
    std::string chunkPath = FileUtils::getChunkPath(chunksDir, filename, chunkIndex);
    ChunkInfo chunkInfo;
    
    if (!NetworkUtils::receiveChunk(socket, chunkPath, chunkInfo)) {
        std::cerr << "Failed to receive chunk" << std::endl;
        NetworkUtils::closeSocket(socket);
        return false;
    }
    
    NetworkUtils::closeSocket(socket);
    std::cout << "Successfully downloaded chunk " << chunkIndex << std::endl;
    return true;
}

std::vector<ChunkInfo> P2PClient::requestChunkListFromPeer(const std::string& ip, int port) {
    std::cout << "Requesting chunk list from peer " << ip << ":" << port << std::endl;
    
    std::vector<ChunkInfo> chunks;
    
    // Connect to peer
    SOCKET socket = NetworkUtils::connectToPeer(ip, port);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Failed to connect to peer " << ip << ":" << port << std::endl;
        return chunks;
    }
    
    // Send chunk list request
    MessageHeader requestHeader;
    requestHeader.type = MSG_CHUNK_LIST_REQUEST;
    requestHeader.dataSize = 0;
    
    if (!NetworkUtils::sendMessage(socket, requestHeader)) {
        std::cerr << "Failed to send chunk list request" << std::endl;
        NetworkUtils::closeSocket(socket);
        return chunks;
    }
    
    // Receive chunk list
    if (!NetworkUtils::receiveChunkList(socket, chunks)) {
        std::cerr << "Failed to receive chunk list" << std::endl;
        NetworkUtils::closeSocket(socket);
        return std::vector<ChunkInfo>();
    }
    
    NetworkUtils::closeSocket(socket);
    std::cout << "Received " << chunks.size() << " chunks from peer" << std::endl;
    
    // Update peer manager with chunk information
    peerManager.updatePeerChunks(ip, port, chunks);
    
    return chunks;
}

bool P2PClient::pingPeer(const std::string& ip, int port) {
    return peerManager.pingPeer(ip, port);
}

std::vector<Peer> P2PClient::findPeersWithFile(const std::string& filename) {
    std::vector<Peer> peersWithFile;
    std::vector<Peer> allPeers = peerManager.getActivePeers();
    
    std::cout << "Searching for file among " << allPeers.size() << " peers..." << std::endl;
    
    for (auto& peer : allPeers) {
        // Request chunk list from peer
        std::vector<ChunkInfo> chunks = requestChunkListFromPeer(peer.ip, peer.port);
        
        // Check if peer has any chunks of this file
        for (const auto& chunk : chunks) {
            if (chunk.filename == filename) {
                peersWithFile.push_back(peer);
                std::cout << "Peer " << peer.ip << ":" << peer.port << " has the file" << std::endl;
                break;
            }
        }
    }
    
    return peersWithFile;
}

bool P2PClient::downloadChunk(const Peer& peer, const std::string& filename, int chunkIndex) {
    return requestChunkFromPeer(peer.ip, peer.port, filename, chunkIndex);
}

void P2PClient::assignChunksToPeers(const std::string& filename, int totalChunks,
                                   const std::vector<Peer>& peers,
                                   std::vector<std::pair<Peer, int>>& assignments) {
    assignments.clear();
    
    if (peers.empty()) {
        std::cerr << "No peers available for chunk assignment" << std::endl;
        return;
    }
    
    std::cout << "Assigning " << totalChunks << " chunks to " << peers.size() << " peers (round-robin)" << std::endl;
    
    // Round-robin assignment
    for (int chunkIndex = 0; chunkIndex < totalChunks; chunkIndex++) {
        int peerIndex = chunkIndex % peers.size();
        assignments.push_back(std::make_pair(peers[peerIndex], chunkIndex));
        
        std::cout << "  Chunk " << chunkIndex << " -> Peer " << peers[peerIndex].ip 
                  << ":" << peers[peerIndex].port << std::endl;
    }
}

bool P2PClient::mergeDownloadedChunks(const std::string& filename, int totalChunks) {
    std::cout << "Merging " << totalChunks << " chunks..." << std::endl;
    
    // Build chunk info list
    std::vector<ChunkInfo> chunks;
    for (int i = 0; i < totalChunks; i++) {
        std::string chunkPath = FileUtils::getChunkPath(chunksDir, filename, i);
        
        if (!FileUtils::fileExists(chunkPath)) {
            std::cerr << "Missing chunk " << i << ": " << chunkPath << std::endl;
            return false;
        }
        
        size_t chunkSize = FileUtils::getFileSize(chunkPath);
        ChunkInfo chunk(filename, i, chunkSize);
        chunks.push_back(chunk);
    }
    
    // Merge chunks
    std::string outputFile = downloadDir + "/" + filename;
    if (!FileUtils::mergeChunksFromInfo(chunks, chunksDir, outputFile)) {
        std::cerr << "Failed to merge chunks" << std::endl;
        return false;
    }
    
    std::cout << "Successfully merged file: " << outputFile << std::endl;
    return true;
}

void P2PClient::updateDownloadProgress(const std::string& filename, int completedChunks, int totalChunks) {
    float progress = (float)completedChunks / totalChunks * 100.0f;
    std::cout << "Download progress for " << filename << ": " 
              << completedChunks << "/" << totalChunks 
              << " (" << progress << "%)" << std::endl;
}

// Static function for chunk download task
void P2PClient::downloadChunkTask(ChunkDownloadTask* task) {
    if (task == nullptr) {
        return;
    }
    
    std::cout << "[Thread] Downloading chunk " << task->chunkIndex << " from " 
              << task->ip << ":" << task->port << std::endl;
    
    // Connect to peer
    SOCKET socket = NetworkUtils::connectToPeer(task->ip, task->port);
    if (socket == INVALID_SOCKET) {
        std::cerr << "[Thread] Failed to connect to peer for chunk " << task->chunkIndex << std::endl;
        task->success = false;
        return;
    }
    
    // Send chunk request
    if (!NetworkUtils::sendChunkRequest(socket, task->filename, task->chunkIndex)) {
        std::cerr << "[Thread] Failed to send request for chunk " << task->chunkIndex << std::endl;
        NetworkUtils::closeSocket(socket);
        task->success = false;
        return;
    }
    
    // Receive chunk
    ChunkInfo chunkInfo;
    if (!NetworkUtils::receiveChunk(socket, task->outputPath, chunkInfo)) {
        std::cerr << "[Thread] Failed to receive chunk " << task->chunkIndex << std::endl;
        NetworkUtils::closeSocket(socket);
        task->success = false;
        return;
    }
    
    NetworkUtils::closeSocket(socket);
    std::cout << "[Thread] Successfully downloaded chunk " << task->chunkIndex << std::endl;
    task->success = true;
}

bool P2PClient::downloadChunksParallel(const std::vector<ChunkDownloadTask>& tasks) {
    if (tasks.empty()) {
        return true;
    }
    
    std::cout << "Starting parallel download of " << tasks.size() << " chunks..." << std::endl;
    
    // For simplicity, download sequentially for now
    // In a full implementation, we would use threads here
    int successCount = 0;
    
    for (size_t i = 0; i < tasks.size(); i++) {
        ChunkDownloadTask task = tasks[i];
        downloadChunkTask(&task);
        
        if (task.success) {
            successCount++;
            updateDownloadProgress(task.filename, successCount, static_cast<int>(tasks.size()));
        } else {
            std::cerr << "Failed to download chunk " << task.chunkIndex << std::endl;
        }
    }
    
    std::cout << "Downloaded " << successCount << "/" << tasks.size() << " chunks" << std::endl;
    return successCount == static_cast<int>(tasks.size());
}