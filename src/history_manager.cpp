#include "../include/history_manager.h"
#include "../include/file_utils.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

const std::string HistoryManager::HISTORY_FILE = "data/history.txt";

std::string HistoryManager::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool HistoryManager::appendToHistoryFile(const std::string& entry) {
    // Ensure data directory exists
    if (!FileUtils::directoryExists("data")) {
        FileUtils::createDirectory("data");
    }
    
    std::ofstream file(HISTORY_FILE, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "[HISTORY] Failed to open history file: " << HISTORY_FILE << std::endl;
        return false;
    }
    
    file << getCurrentTimestamp() << " | " << entry << std::endl;
    file.close();
    
    std::cout << "[HISTORY] " << entry << std::endl;
    return true;
}

bool HistoryManager::logFileShared(const std::string& filename, const std::string& toIp, int chunks) {
    std::stringstream ss;
    ss << "SHARED: " << filename << " to " << toIp << " (" << chunks << " chunks)";
    return appendToHistoryFile(ss.str());
}

bool HistoryManager::logFileReceived(const std::string& filename, const std::string& fromIp, int chunks) {
    std::stringstream ss;
    ss << "RECEIVED: " << filename << " from " << fromIp << " (" << chunks << " chunks)";
    return appendToHistoryFile(ss.str());
}

bool HistoryManager::logChunkTransfer(const std::string& filename, const std::string& peerIp, 
                                     int chunkIndex, const std::string& direction) {
    std::stringstream ss;
    if (direction == "sent") {
        ss << "CHUNK_SENT: " << filename << " chunk " << chunkIndex << " to " << peerIp;
    } else {
        ss << "CHUNK_RECEIVED: " << filename << " chunk " << chunkIndex << " from " << peerIp;
    }
    return appendToHistoryFile(ss.str());
}

std::vector<std::string> HistoryManager::getHistory(int maxLines) {
    std::vector<std::string> history;
    std::ifstream file(HISTORY_FILE);
    
    if (!file.is_open()) {
        return history; // Return empty vector if file doesn't exist
    }
    
    std::string line;
    std::vector<std::string> allLines;
    
    // Read all lines
    while (std::getline(file, line)) {
        allLines.push_back(line);
    }
    file.close();
    
    // Return last maxLines entries (most recent first)
    int startIndex = std::max(0, (int)allLines.size() - maxLines);
    for (int i = allLines.size() - 1; i >= startIndex; i--) {
        history.push_back(allLines[i]);
    }
    
    return history;
}

bool HistoryManager::clearHistory() {
    std::ofstream file(HISTORY_FILE, std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.close();
    std::cout << "[HISTORY] History cleared" << std::endl;
    return true;
}