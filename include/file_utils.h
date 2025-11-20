#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>
#include "peer.h"

// File operations class
class FileUtils {
public:
    // File splitting functions
    static std::vector<std::string> splitFile(const std::string& filePath, 
                                             int chunkSize, 
                                             const std::string& outputDir);
    
    static bool splitFileToChunks(const std::string& filePath,
                                 const std::string& outputDir,
                                 std::vector<ChunkInfo>& chunkList);
    
    // File merging functions
    static bool mergeChunks(const std::vector<std::string>& chunkFiles, 
                           const std::string& outputFile);
    
    static bool mergeChunksFromInfo(const std::vector<ChunkInfo>& chunks,
                                   const std::string& chunksDir,
                                   const std::string& outputFile);
    
    // File integrity functions
    static std::string computeFileHash(const std::string& filePath);
    static std::string computeChunkHash(const std::string& chunkPath);
    static bool verifyChunkIntegrity(const std::string& chunkPath, 
                                    const std::string& expectedHash);
    
    // File information functions
    static size_t getFileSize(const std::string& filePath);
    static bool fileExists(const std::string& filePath);
    static int calculateChunkCount(size_t fileSize, int chunkSize);
    
    // Chunk file naming
    static std::string generateChunkName(const std::string& filename, int chunkIndex);
    static std::string getChunkPath(const std::string& chunksDir, 
                                   const std::string& filename, 
                                   int chunkIndex);
    
    // Directory operations
    static bool createDirectory(const std::string& dirPath);
    static bool directoryExists(const std::string& dirPath);
    static std::vector<std::string> listFiles(const std::string& dirPath);
    
    // Cleanup operations
    static bool deleteChunkFiles(const std::string& chunksDir, const std::string& filename);
    static bool cleanupTempFiles(const std::string& tempDir);
    
    // Advanced file operations
    static bool verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash);
    static bool compareFiles(const std::string& file1, const std::string& file2);
    static std::string getFileExtension(const std::string& filename);
    static std::string getBaseName(const std::string& filename);
    
    // Chunk tracking
    static bool saveChunkMetadata(const std::vector<ChunkInfo>& chunks, const std::string& metaFile);
    static bool loadChunkMetadata(std::vector<ChunkInfo>& chunks, const std::string& metaFile);
    static bool isChunkComplete(const std::string& chunkPath, const ChunkInfo& expectedInfo);

private:
    // Helper functions
    static std::string bytesToHex(const unsigned char* bytes, size_t length);
    static bool copyFileChunk(const std::string& sourcePath, 
                             const std::string& destPath,
                             size_t offset, 
                             size_t size);
};

#endif // FILE_UTILS_H