# 🧠 Decentralized Peer-to-Peer File Sharing System in C++

## 🗂 Project Overview
This is a **fully functional** Decentralized Peer-to-Peer (P2P) File Sharing System built in C++, demonstrating distributed systems, socket programming, file chunking, and decentralized communication.

**✅ PROJECT STATUS: COMPLETE AND READY TO USE**

Share files directly between laptops on the same network without any central server. Each peer acts as both a client (requesting files) and a server (providing files). The system enables parallel file downloading where a single file is split into chunks and downloaded from multiple peers simultaneously.

## 🚀 Quick Start

### 1. Compile the Project
```bash
mingw32-make    # Windows with MinGW
make            # Linux/Mac
```

### 2. Run Quick Setup
```bash
QUICK_START.bat    # Windows
```

### 3. Share a File (Laptop A)
```bash
# Put your file in data/shared/
p2p_share.exe --mode=share --file=document.pdf --port=8080
```

### 4. Download File (Laptop B)
```bash
# Edit peers.txt to add Laptop A's IP:PORT
# Then download:
p2p_share.exe --mode=download --file=document.pdf
```

**📖 See [USER_GUIDE.md](USER_GUIDE.md) for detailed instructions**

## 🎯 What This System Does
- ✅ Share files between multiple computers on LAN
- ✅ Split large files into chunks automatically
- ✅ Download chunks in parallel from multiple peers
- ✅ Reconstruct files perfectly after download
- ✅ Interactive menu for easy file management
- ✅ No central server required
- ✅ Works on Windows, Linux, and Mac

## ⚙ Core Features
- ✅ **Peer Discovery**: Peers identified by IP and port via peers.txt
- ✅ **Decentralization**: No central server - every node acts as both client and server
- ✅ **File Chunking**: Files divided into 1MB chunks automatically
- ✅ **Parallel Downloading**: Chunks downloaded from multiple peers simultaneously
- ✅ **File Reassembly**: Chunks merged perfectly to reconstruct original file
- ✅ **LAN Functionality**: Works entirely over local Wi-Fi or LAN
- ✅ **Multi-threaded Server**: Handles multiple client connections concurrently
- ✅ **Interactive Mode**: User-friendly menu for file management
- ✅ **File Integrity**: Hash verification ensures file correctness

## 💡 Real-World Example

**Scenario:** Share a 50MB presentation between 3 laptops in a meeting room

**Laptop A (192.168.1.100):**
```bash
p2p_share.exe --mode=share --file=presentation.pptx --port=8080
# File split into 50 chunks, server running
```

**Laptop B & C:**
```bash
# Add to peers.txt: 192.168.1.100:8080
p2p_share.exe --mode=download --file=presentation.pptx
# Downloads 50 chunks in parallel, merges into final file
```

**Result:** File shared in seconds without USB drives or cloud storage!

## 🏗 Directory Structure
```
P2PFileShare/
│
├── include/
│   ├── peer.h              # Defines Peer structure and chunk list handling
│   ├── file_utils.h        # File splitting, hashing, and merging functions
│   ├── network_utils.h     # Socket creation and communication functions
│
├── src/
│   ├── main.cpp            # Entry point and flow control
│   ├── server.cpp          # Handles incoming chunk requests
│   ├── client.cpp          # Requests chunks from peers in parallel
│   ├── peer.cpp            # Peer data exchange and chunk list synchronization
│   ├── file_utils.cpp      # Implementation for split/merge logic
│   ├── network_utils.cpp   # Implementation for socket functions
│
├── data/
│   ├── shared/             # Files available for sharing
│   ├── chunks/             # Temporary chunk storage
│   ├── downloads/          # Merged downloaded files
│
├── peers.txt               # IP:Port list of known peers
├── Makefile                # For compilation
└── README.md               # Documentation
```

## 🚀 Implementation Steps

### Phase 1: Project Setup and Basic Structure
1. **Initialize Project Structure**
   - Create all directories as shown above
   - Set up basic header files with function declarations
   - Create empty source files with basic includes

2. **Configure Build System**
   - Write Makefile with proper compilation flags
   - Include necessary libraries (pthread, socket libraries)
   - Set up debug and release build targets

3. **Define Core Data Structures**
   - Create Peer struct in peer.h
   - Define chunk metadata structure
   - Set up configuration constants (chunk size, buffer sizes)

### Phase 2: Network Foundation
4. **Implement Basic Socket Operations**
   - Create server socket binding and listening functions
   - Implement client connection establishment
   - Add basic send/receive data functions
   - Handle socket errors and cleanup

5. **Develop Peer Communication Protocol**
   - Design message format for peer communication
   - Implement peer discovery mechanism
   - Create functions to exchange chunk availability lists
   - Add peer list management (add/remove/update peers)

### Phase 3: File Management System
6. **Build File Chunking System**
   - Implement file splitting into fixed-size chunks
   - Create chunk naming convention
   - Add chunk metadata tracking
   - Implement file size and chunk count calculations

7. **Develop File Merging Capability**
   - Create chunk reassembly function
   - Implement file integrity verification
   - Add error handling for missing or corrupted chunks
   - Create temporary file management for partial downloads

### Phase 4: Server Implementation
8. **Create Peer Server Component**
   - Implement multi-threaded server to handle multiple requests
   - Add request parsing for chunk requests
   - Create chunk serving functionality
   - Implement connection management and cleanup

9. **Add Chunk Availability Broadcasting**
   - Implement periodic chunk list updates to peers
   - Create mechanism to announce new files/chunks
   - Add handling for peer status updates

### Phase 5: Client Implementation
10. **Build Download Client**
    - Implement peer discovery and connection
    - Create chunk request distribution algorithm
    - Add parallel download management using threads
    - Implement download progress tracking

11. **Develop Chunk Assignment Strategy**
    - Create round-robin or random chunk assignment
    - Implement load balancing across peers
    - Add retry mechanism for failed downloads
    - Handle peer disconnections gracefully

### Phase 6: Main Application Logic
12. **Create Main Application Controller**
    - Implement user interface for share/download modes
    - Add file selection and management
    - Create application startup and shutdown procedures
    - Implement configuration loading

13. **Add Application Modes**
    - Share mode: Split files and start server
    - Download mode: Request files and merge chunks
    - Hybrid mode: Simultaneous sharing and downloading

### Phase 7: Error Handling and Robustness
14. **Implement Comprehensive Error Handling**
    - Add network error recovery
    - Implement file I/O error handling
    - Create timeout mechanisms for stalled transfers
    - Add logging system for debugging

15. **Add Data Integrity Features**
    - Implement chunk checksums (MD5/SHA256)
    - Add chunk verification during download
    - Create corrupted chunk re-download mechanism
    - Implement file completion verification

### Phase 8: Testing and Optimization
16. **Create Testing Framework**
    - Write unit tests for core functions
    - Create integration tests for peer communication
    - Add performance benchmarking
    - Test with multiple peers and large files

17. **Performance Optimization**
    - Optimize buffer sizes and chunk sizes
    - Implement connection pooling
    - Add bandwidth throttling options
    - Optimize memory usage for large files

### Phase 9: Documentation and Polish
18. **Complete Documentation**
    - Write detailed API documentation
    - Create user manual with examples
    - Add troubleshooting guide
    - Document network requirements and setup

19. **Final Testing and Validation**
    - Test on different network configurations
    - Validate with various file types and sizes
    - Perform stress testing with multiple concurrent users
    - Create demonstration scenarios

## 🧪 Testing Scenarios

### Basic Functionality Test
1. **Single Peer Sharing**: One peer shares a file, another downloads
2. **Multi-Peer Download**: Download file chunks from 3+ different peers
3. **Concurrent Operations**: Multiple peers downloading different files simultaneously
4. **Network Interruption**: Test recovery when peers disconnect mid-transfer

### Advanced Testing
1. **Large File Handling**: Test with files > 100MB
2. **Many Small Files**: Test system with numerous small files
3. **Peer Discovery**: Test automatic peer detection and list updates
4. **Load Testing**: Simulate high concurrent download requests

## 🔧 Development Tips

### Implementation Order Priority
1. Start with basic file operations (split/merge)
2. Implement simple socket communication
3. Create single-threaded server/client
4. Add multi-threading support
5. Implement peer discovery and management
6. Add error handling and robustness features

### Common Pitfalls to Avoid
- **Memory Leaks**: Properly manage socket and file handles
- **Thread Safety**: Use proper synchronization for shared data
- **Network Endianness**: Handle byte order for cross-platform compatibility
- **Buffer Overflows**: Validate all input data sizes
- **Deadlocks**: Careful ordering of lock acquisition

### Debugging Strategies
- Use extensive logging for network operations
- Implement verbose mode for troubleshooting
- Create network packet capture capabilities
- Add performance timing measurements

## 🎓 Learning Outcomes
- Understanding TCP socket programming in C++
- Implementing multi-threading for parallel data transfer
- Grasping decentralized architecture and peer equality
- Applying file I/O operations and data integrity validation
- Real-world simulation of BitTorrent-like protocols

## 💻 Usage Example
```bash
# Compile the project
make

# Run as server/seeder
./p2p_share --mode=share --file=document.pdf --port=8080

# Run as downloader
./p2p_share --mode=download --file=document.pdf --port=8081

# Run in hybrid mode
./p2p_share --mode=hybrid --port=8082
```

## 🔮 Future Enhancements
- Automatic peer discovery via UDP broadcast
- Checksum-based integrity verification
- Chunk redundancy for fault tolerance
- GUI frontend for easier usage
- Encryption for secure data transmission
- DHT (Distributed Hash Table) implementation
- Bandwidth management and QoS
- Resume interrupted downloads

## 📊 Project Metrics
- **Language**: C++
- **Concepts**: Socket Programming, Multithreading, File I/O, Decentralization
- **Architecture**: Fully Peer-to-Peer (No Central Server)
- **Network Type**: Local Network (LAN/Wi-Fi)
- **Difficulty Level**: Moderate to Advanced
- **Target Audience**: Students learning networking and distributed systems

---

**One-Line Summary**: A C++ project implementing a decentralized peer-to-peer file sharing network where each node acts as both server and client, enabling multi-peer parallel chunk-based file transfer and reassembly without a central server — built using socket programming, file I/O, and threading.