#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "../include/peer.h"
#include "../include/network_utils.h"

// PeerManager implementation
bool PeerManager::loadPeersFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open peers file: " << filename << std::endl;
        return false;
    }
    
    peers.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        std::istringstream iss(line);
        std::string ip;
        int port;
        
        if (std::getline(iss, ip, ':') && (iss >> port)) {
            Peer peer(ip, port);
            peers.push_back(peer);
            std::cout << "Loaded peer: " << ip << ":" << port << std::endl;
        }
    }
    
    file.close();
    std::cout << "Loaded " << peers.size() << " peers from " << filename << std::endl;
    return true;
}

bool PeerManager::savePeersToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open peers file for writing: " << filename << std::endl;
        return false;
    }
    
    file << "# Peer list - format: IP:PORT\n";
    for (const auto& peer : peers) {
        file << peer.ip << ":" << peer.port << "\n";
    }
    
    file.close();
    std::cout << "Saved " << peers.size() << " peers to " << filename << std::endl;
    return true;
}

void PeerManager::addPeer(const Peer& peer) {
    // Check if peer already exists
    auto it = std::find(peers.begin(), peers.end(), peer);
    if (it == peers.end()) {
        peers.push_back(peer);
        std::cout << "Added new peer: " << peer.ip << ":" << peer.port << std::endl;
    } else {
        // Update existing peer status
        it->isOnline = peer.isOnline;
        std::cout << "Updated peer status: " << peer.ip << ":" << peer.port 
                  << " (online: " << peer.isOnline << ")" << std::endl;
    }
}

void PeerManager::removePeer(const std::string& ip, int port) {
    Peer targetPeer(ip, port);
    auto it = std::find(peers.begin(), peers.end(), targetPeer);
    if (it != peers.end()) {
        peers.erase(it);
        std::cout << "Removed peer: " << ip << ":" << port << std::endl;
    }
}

std::vector<Peer> PeerManager::getActivePeers() {
    std::vector<Peer> activePeers;
    for (const auto& peer : peers) {
        if (peer.isOnline) {
            activePeers.push_back(peer);
        }
    }
    return activePeers;
}

void PeerManager::updatePeerChunks(const std::string& ip, int port, 
                                  const std::vector<ChunkInfo>& chunks) {
    for (auto& peer : peers) {
        if (peer.ip == ip && peer.port == port) {
            peer.availableChunks = chunks;
            std::cout << "Updated chunks for peer " << ip << ":" << port 
                      << " (" << chunks.size() << " chunks)" << std::endl;
            return;
        }
    }
    
    // If peer not found, add it
    Peer newPeer(ip, port);
    newPeer.availableChunks = chunks;
    addPeer(newPeer);
}

std::vector<ChunkInfo> PeerManager::getPeerChunks(const std::string& ip, int port) {
    for (const auto& peer : peers) {
        if (peer.ip == ip && peer.port == port) {
            return peer.availableChunks;
        }
    }
    return std::vector<ChunkInfo>();
}

std::vector<Peer> PeerManager::getPeersWithChunk(const std::string& filename, int chunkIndex) {
    std::vector<Peer> peersWithChunk;
    
    for (const auto& peer : peers) {
        if (!peer.isOnline) continue;
        
        for (const auto& chunk : peer.availableChunks) {
            if (chunk.filename == filename && chunk.chunkIndex == chunkIndex && chunk.isAvailable) {
                peersWithChunk.push_back(peer);
                break;
            }
        }
    }
    
    return peersWithChunk;
}

bool PeerManager::sendChunkList(const std::string& ip, int port, 
                               const std::vector<ChunkInfo>& chunks) {
    SOCKET socket = NetworkUtils::connectToPeer(ip, port);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Failed to connect to peer " << ip << ":" << port << std::endl;
        return false;
    }
    
    bool success = NetworkUtils::sendChunkList(socket, chunks);
    NetworkUtils::closeSocket(socket);
    
    std::cout << "Sent chunk list to peer " << ip << ":" << port 
              << " (" << chunks.size() << " chunks)" << std::endl;
    return success;
}

bool PeerManager::requestChunkList(const std::string& ip, int port, 
                                  std::vector<ChunkInfo>& chunks) {
    SOCKET socket = NetworkUtils::connectToPeer(ip, port);
    if (socket == INVALID_SOCKET) {
        std::cerr << "Failed to connect to peer " << ip << ":" << port << std::endl;
        return false;
    }
    
    // Send chunk list request
    MessageHeader requestHeader;
    requestHeader.type = MSG_CHUNK_LIST_REQUEST;
    requestHeader.dataSize = 0;
    
    if (!NetworkUtils::sendMessage(socket, requestHeader)) {
        std::cerr << "Failed to send chunk list request" << std::endl;
        NetworkUtils::closeSocket(socket);
        return false;
    }
    
    // Receive chunk list
    bool success = NetworkUtils::receiveChunkList(socket, chunks);
    NetworkUtils::closeSocket(socket);
    
    if (success) {
        std::cout << "Received chunk list from peer " << ip << ":" << port 
                  << " (" << chunks.size() << " chunks)" << std::endl;
        updatePeerChunks(ip, port, chunks);
    }
    
    return success;
}

bool PeerManager::pingPeer(const std::string& ip, int port) {
    SOCKET socket = NetworkUtils::connectToPeer(ip, port);
    if (socket == INVALID_SOCKET) {
        // Update peer status as offline
        for (auto& peer : peers) {
            if (peer.ip == ip && peer.port == port) {
                peer.isOnline = false;
                break;
            }
        }
        return false;
    }
    
    // Send ping message
    MessageHeader pingHeader;
    pingHeader.type = MSG_PEER_PING;
    pingHeader.dataSize = 0;
    
    if (!NetworkUtils::sendMessage(socket, pingHeader)) {
        NetworkUtils::closeSocket(socket);
        return false;
    }
    
    // Wait for pong response
    MessageHeader pongHeader;
    std::vector<char> data;
    
    bool success = NetworkUtils::receiveMessage(socket, pongHeader, data);
    NetworkUtils::closeSocket(socket);
    
    if (success && pongHeader.type == MSG_PEER_PONG) {
        // Update peer status as online
        for (auto& peer : peers) {
            if (peer.ip == ip && peer.port == port) {
                peer.isOnline = true;
                break;
            }
        }
        std::cout << "Peer " << ip << ":" << port << " is online" << std::endl;
        return true;
    }
    
    // Update peer status as offline
    for (auto& peer : peers) {
        if (peer.ip == ip && peer.port == port) {
            peer.isOnline = false;
            break;
        }
    }
    return false;
}