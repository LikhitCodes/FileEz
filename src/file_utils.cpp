#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define mkdir(path, mode) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <dirent.h>
#endif
#include "../include/file_utils.h"

// Helper function to extract filename from path
std::string extractFilename(const std::string& filePath) {
    std::string filename = filePath;
    size_t lastSlash = filename.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filename.substr(lastSlash + 1);
    }
    return filename;
}

// Static member implementations
std::vector<std::string> FileUtils::splitFile(const std::string& filePath, 
                                             int chunkSize, 
                                             const std::string& outputDir) {
    std::vector<std::string> chunkFiles;
    
    std::ifstream inputFile(filePath.c_str(), std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Could not open file for reading: " << filePath << std::endl;
        return chunkFiles;
    }
    
    // Create output directory if it doesn't exist
    if (!createDirectory(outputDir)) {
        std::cerr << "Could not create output directory: " << outputDir << std::endl;
        return chunkFiles;
    }
    
    std::string filename = extractFilename(filePath);
    std::vector<char> buffer(chunkSize);
    int chunkIndex = 0;
    
    while (inputFile.read(buffer.data(), chunkSize) || inputFile.gcount() > 0) {
        size_t bytesRead = static_cast<size_t>(inputFile.gcount());
        
        std::string chunkFilename = generateChunkName(filename, chunkIndex);
        std::string chunkPath = outputDir + "/" + chunkFilename;
        
        std::ofstream chunkFile(chunkPath.c_str(), std::ios::binary);
        if (chunkFile.is_open()) {
            chunkFile.write(buffer.data(), static_cast<std::streamsize>(bytesRead));
            chunkFile.close();
            chunkFiles.push_back(chunkPath);
            
            std::cout << "Created chunk " << chunkIndex << ": " << chunkPath 
                      << " (" << bytesRead << " bytes)" << std::endl;
        } else {
            std::cerr << "Could not create chunk file: " << chunkPath << std::endl;
        }
        
        chunkIndex++;
    }
    
    inputFile.close();
    std::cout << "Split file into " << chunkFiles.size() << " chunks" << std::endl;
    return chunkFiles;
}

bool FileUtils::splitFileToChunks(const std::string& filePath,
                                 const std::string& outputDir,
                                 std::vector<ChunkInfo>& chunkList) {
    chunkList.clear();
    
    size_t fileSize = getFileSize(filePath);
    if (fileSize == 0) {
        std::cerr << "File is empty or does not exist: " << filePath << std::endl;
        return false;
    }
    
    std::string filename = extractFilename(filePath);
    
    std::vector<std::string> chunkFiles = splitFile(filePath, DEFAULT_CHUNK_SIZE, outputDir);
    
    for (size_t i = 0; i < chunkFiles.size(); i++) {
        ChunkInfo chunk(filename, static_cast<int>(i), getFileSize(chunkFiles[i]));
        chunk.checksum = computeChunkHash(chunkFiles[i]);
        chunkList.push_back(chunk);
    }
    
    return !chunkList.empty();
}

bool FileUtils::mergeChunks(const std::vector<std::string>& chunkFiles, 
                           const std::string& outputFile) {
    std::ofstream output(outputFile.c_str(), std::ios::binary);
    if (!output.is_open()) {
        std::cerr << "Could not create output file: " << outputFile << std::endl;
        return false;
    }
    
    for (const auto& chunkFile : chunkFiles) {
        std::ifstream chunk(chunkFile.c_str(), std::ios::binary);
        if (!chunk.is_open()) {
            std::cerr << "Could not open chunk file: " << chunkFile << std::endl;
            output.close();
            return false;
        }
        
        output << chunk.rdbuf();
        chunk.close();
        
        std::cout << "Merged chunk: " << chunkFile << std::endl;
    }
    
    output.close();
    std::cout << "Successfully merged " << chunkFiles.size() << " chunks into " << outputFile << std::endl;
    return true;
}

bool FileUtils::mergeChunksFromInfo(const std::vector<ChunkInfo>& chunks,
                                   const std::string& chunksDir,
                                   const std::string& outputFile) {
    // Sort chunks by index to ensure correct order
    std::vector<ChunkInfo> sortedChunks = chunks;
    std::sort(sortedChunks.begin(), sortedChunks.end(), 
              [](const ChunkInfo& a, const ChunkInfo& b) {
                  return a.chunkIndex < b.chunkIndex;
              });
    
    std::vector<std::string> chunkFiles;
    for (const auto& chunk : sortedChunks) {
        std::string chunkPath = getChunkPath(chunksDir, chunk.filename, chunk.chunkIndex);
        chunkFiles.push_back(chunkPath);
    }
    
    return mergeChunks(chunkFiles, outputFile);
}

std::string FileUtils::computeFileHash(const std::string& filePath) {
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    // Simple but effective hash: read file in chunks and compute rolling hash
    const size_t BUFFER_SIZE = 8192;
    std::vector<char> buffer(BUFFER_SIZE);
    unsigned long hash = 5381; // DJB2 hash initial value
    
    while (file.read(buffer.data(), BUFFER_SIZE) || file.gcount() > 0) {
        size_t bytesRead = static_cast<size_t>(file.gcount());
        for (size_t i = 0; i < bytesRead; i++) {
            hash = ((hash << 5) + hash) + static_cast<unsigned char>(buffer[i]); // hash * 33 + c
        }
    }
    
    file.close();
    
    // Convert to hex string
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return ss.str();
}

std::string FileUtils::computeChunkHash(const std::string& chunkPath) {
    return computeFileHash(chunkPath);
}

bool FileUtils::verifyChunkIntegrity(const std::string& chunkPath, 
                                    const std::string& expectedHash) {
    std::string actualHash = computeChunkHash(chunkPath);
    return actualHash == expectedHash;
}

size_t FileUtils::getFileSize(const std::string& filePath) {
    std::ifstream file(filePath.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return 0;
    }
    
    std::streampos size = file.tellg();
    file.close();
    return static_cast<size_t>(size);
}

bool FileUtils::fileExists(const std::string& filePath) {
    std::ifstream file(filePath.c_str());
    return file.good();
}

int FileUtils::calculateChunkCount(size_t fileSize, int chunkSize) {
    return static_cast<int>((fileSize + chunkSize - 1) / chunkSize);
}

std::string FileUtils::generateChunkName(const std::string& filename, int chunkIndex) {
    std::stringstream ss;
    ss << filename << ".chunk." << std::setfill('0') << std::setw(4) << chunkIndex;
    return ss.str();
}

std::string FileUtils::getChunkPath(const std::string& chunksDir, 
                                   const std::string& filename, 
                                   int chunkIndex) {
    std::string chunkName = generateChunkName(filename, chunkIndex);
    return chunksDir + "/" + chunkName;
}

bool FileUtils::createDirectory(const std::string& dirPath) {
#ifdef _WIN32
    return CreateDirectoryA(dirPath.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(dirPath.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

bool FileUtils::directoryExists(const std::string& dirPath) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(dirPath.c_str());
    return (attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(dirPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

std::vector<std::string> FileUtils::listFiles(const std::string& dirPath) {
    std::vector<std::string> files;
    
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string searchPath = dirPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(dirPath + "\\" + findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(dirPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {
                files.push_back(dirPath + "/" + entry->d_name);
            }
        }
        closedir(dir);
    }
#endif
    
    return files;
}

bool FileUtils::deleteChunkFiles(const std::string& chunksDir, const std::string& filename) {
    std::vector<std::string> files = listFiles(chunksDir);
    int deletedCount = 0;
    
    // Find and delete all chunk files for this filename
    for (const auto& file : files) {
        std::string fileOnly = extractFilename(file);
        
        // Check if this file is a chunk of the target file
        if (fileOnly.find(filename + ".chunk.") == 0) {
#ifdef _WIN32
            if (DeleteFileA(file.c_str())) {
                deletedCount++;
                std::cout << "Deleted chunk: " << file << std::endl;
            }
#else
            if (remove(file.c_str()) == 0) {
                deletedCount++;
                std::cout << "Deleted chunk: " << file << std::endl;
            }
#endif
        }
    }
    
    std::cout << "Deleted " << deletedCount << " chunk files for " << filename << std::endl;
    return deletedCount > 0;
}

bool FileUtils::cleanupTempFiles(const std::string& tempDir) {
    if (!directoryExists(tempDir)) {
        std::cerr << "Directory does not exist: " << tempDir << std::endl;
        return false;
    }
    
    std::vector<std::string> files = listFiles(tempDir);
    int deletedCount = 0;
    
    for (const auto& file : files) {
        // Delete all files in the temp directory
#ifdef _WIN32
        if (DeleteFileA(file.c_str())) {
            deletedCount++;
        }
#else
        if (remove(file.c_str()) == 0) {
            deletedCount++;
        }
#endif
    }
    
    std::cout << "Cleaned up " << deletedCount << " temporary files from " << tempDir << std::endl;
    return true;
}

std::string FileUtils::bytesToHex(const unsigned char* bytes, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; i++) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

bool FileUtils::copyFileChunk(const std::string& sourcePath, 
                             const std::string& destPath,
                             size_t offset, 
                             size_t size) {
    std::ifstream source(sourcePath.c_str(), std::ios::binary);
    if (!source.is_open()) {
        std::cerr << "Could not open source file: " << sourcePath << std::endl;
        return false;
    }
    
    // Seek to offset
    source.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!source.good()) {
        std::cerr << "Could not seek to offset " << offset << " in " << sourcePath << std::endl;
        source.close();
        return false;
    }
    
    std::ofstream dest(destPath.c_str(), std::ios::binary);
    if (!dest.is_open()) {
        std::cerr << "Could not create destination file: " << destPath << std::endl;
        source.close();
        return false;
    }
    
    // Copy data in chunks
    const size_t BUFFER_SIZE = 8192;
    std::vector<char> buffer(BUFFER_SIZE);
    size_t remaining = size;
    
    while (remaining > 0 && source.good()) {
        size_t toRead = std::min(BUFFER_SIZE, remaining);
        source.read(buffer.data(), static_cast<std::streamsize>(toRead));
        size_t bytesRead = static_cast<size_t>(source.gcount());
        
        if (bytesRead > 0) {
            dest.write(buffer.data(), static_cast<std::streamsize>(bytesRead));
            remaining -= bytesRead;
        } else {
            break;
        }
    }
    
    source.close();
    dest.close();
    
    std::cout << "Copied " << (size - remaining) << " bytes from " << sourcePath 
              << " to " << destPath << std::endl;
    return remaining == 0;
}

// Advanced file operations
bool FileUtils::verifyFileIntegrity(const std::string& filePath, const std::string& expectedHash) {
    if (!fileExists(filePath)) {
        std::cerr << "File does not exist: " << filePath << std::endl;
        return false;
    }
    
    std::string actualHash = computeFileHash(filePath);
    bool isValid = (actualHash == expectedHash);
    
    if (isValid) {
        std::cout << "File integrity verified: " << filePath << std::endl;
    } else {
        std::cerr << "File integrity check failed for: " << filePath << std::endl;
        std::cerr << "Expected: " << expectedHash << ", Got: " << actualHash << std::endl;
    }
    
    return isValid;
}

bool FileUtils::compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1.c_str(), std::ios::binary);
    std::ifstream f2(file2.c_str(), std::ios::binary);
    
    if (!f1.is_open() || !f2.is_open()) {
        return false;
    }
    
    // Compare file sizes first
    f1.seekg(0, std::ios::end);
    f2.seekg(0, std::ios::end);
    
    if (f1.tellg() != f2.tellg()) {
        f1.close();
        f2.close();
        return false;
    }
    
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    
    // Compare content
    const size_t BUFFER_SIZE = 8192;
    std::vector<char> buffer1(BUFFER_SIZE);
    std::vector<char> buffer2(BUFFER_SIZE);
    
    while (f1.good() && f2.good()) {
        f1.read(buffer1.data(), BUFFER_SIZE);
        f2.read(buffer2.data(), BUFFER_SIZE);
        
        size_t bytes1 = static_cast<size_t>(f1.gcount());
        size_t bytes2 = static_cast<size_t>(f2.gcount());
        
        if (bytes1 != bytes2) {
            f1.close();
            f2.close();
            return false;
        }
        
        if (memcmp(buffer1.data(), buffer2.data(), bytes1) != 0) {
            f1.close();
            f2.close();
            return false;
        }
    }
    
    f1.close();
    f2.close();
    return true;
}

std::string FileUtils::getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < filename.length() - 1) {
        return filename.substr(dotPos + 1);
    }
    return "";
}

std::string FileUtils::getBaseName(const std::string& filename) {
    std::string base = extractFilename(filename);
    size_t dotPos = base.find_last_of('.');
    if (dotPos != std::string::npos) {
        return base.substr(0, dotPos);
    }
    return base;
}

// Chunk tracking
bool FileUtils::saveChunkMetadata(const std::vector<ChunkInfo>& chunks, const std::string& metaFile) {
    std::ofstream file(metaFile.c_str());
    if (!file.is_open()) {
        std::cerr << "Could not create metadata file: " << metaFile << std::endl;
        return false;
    }
    
    // Write header
    file << "# Chunk Metadata File\n";
    file << "# Format: filename,chunkIndex,chunkSize,checksum\n";
    file << "ChunkCount=" << chunks.size() << "\n";
    
    // Write chunk info
    for (const auto& chunk : chunks) {
        file << chunk.filename << ","
             << chunk.chunkIndex << ","
             << chunk.chunkSize << ","
             << chunk.checksum << "\n";
    }
    
    file.close();
    std::cout << "Saved metadata for " << chunks.size() << " chunks to " << metaFile << std::endl;
    return true;
}

bool FileUtils::loadChunkMetadata(std::vector<ChunkInfo>& chunks, const std::string& metaFile) {
    std::ifstream file(metaFile.c_str());
    if (!file.is_open()) {
        std::cerr << "Could not open metadata file: " << metaFile << std::endl;
        return false;
    }
    
    chunks.clear();
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Skip header line
        if (line.find("ChunkCount=") == 0) {
            continue;
        }
        
        // Parse chunk info: filename,chunkIndex,chunkSize,checksum
        std::istringstream iss(line);
        std::string filename, checksum;
        int chunkIndex;
        size_t chunkSize;
        
        if (std::getline(iss, filename, ',')) {
            std::string indexStr, sizeStr;
            if (std::getline(iss, indexStr, ',') &&
                std::getline(iss, sizeStr, ',') &&
                std::getline(iss, checksum)) {
                
                chunkIndex = std::stoi(indexStr);
                chunkSize = static_cast<size_t>(std::stoll(sizeStr));
                
                ChunkInfo chunk(filename, chunkIndex, chunkSize);
                chunk.checksum = checksum;
                chunks.push_back(chunk);
            }
        }
    }
    
    file.close();
    std::cout << "Loaded metadata for " << chunks.size() << " chunks from " << metaFile << std::endl;
    return !chunks.empty();
}

bool FileUtils::isChunkComplete(const std::string& chunkPath, const ChunkInfo& expectedInfo) {
    if (!fileExists(chunkPath)) {
        return false;
    }
    
    // Check file size
    size_t actualSize = getFileSize(chunkPath);
    if (actualSize != expectedInfo.chunkSize) {
        std::cerr << "Chunk size mismatch: expected " << expectedInfo.chunkSize 
                  << ", got " << actualSize << std::endl;
        return false;
    }
    
    // Verify checksum if provided
    if (!expectedInfo.checksum.empty()) {
        return verifyChunkIntegrity(chunkPath, expectedInfo.checksum);
    }
    
    return true;
}
