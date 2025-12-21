#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <string>
#include <vector>

class HistoryManager {
public:
    // Log file transfer events
    static bool logFileShared(const std::string& filename, const std::string& toIp, int chunks);
    static bool logFileReceived(const std::string& filename, const std::string& fromIp, int chunks);
    static bool logChunkTransfer(const std::string& filename, const std::string& peerIp, 
                                int chunkIndex, const std::string& direction); // "sent" or "received"
    
    // Read history
    static std::vector<std::string> getHistory(int maxLines = 100);
    static bool clearHistory();
    
private:
    static std::string getCurrentTimestamp();
    static bool appendToHistoryFile(const std::string& entry);
    static const std::string HISTORY_FILE;
};

#endif // HISTORY_MANAGER_H