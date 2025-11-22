# How This P2P File Sharing System Works

## Table of Contents
1. [Project Architecture Overview](#project-architecture-overview)
2. [Source Files Explained](#source-files-explained)
3. [Deep Dive: server.cpp](#deep-dive-servercpp)
4. [How Files Work Together](#how-files-work-together)
5. [Data Flow Examples](#data-flow-examples)

---

## Project Architecture Overview

### The Big Picture

```
┌─────────────────────────────────────────────────────────┐
│                    P2P File Sharing                      │
│                                                          │
│  ┌──────────┐         ┌──────────┐         ┌─────────┐ │
│  │  main.cpp│────────▶│server.cpp│◀────────│Laptop A │ │
│  │  (Brain) │         │ (Server) │         │ (Peer)  │ │
│  └────┬─────┘         └──────────┘         └─────────┘ │
│       │                                                  │
│       │               ┌──────────┐         ┌─────────┐ │
│       └──────────────▶│client.cpp│────────▶│Laptop B │ │
│                       │ (Client) │         │ (Peer)  │ │
│                       └──────────┘         └─────────┘ │
│                                                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │ file_utils   │  │ network_utils│  │    peer      │ │
│  │ (File Ops)   │  │ (Networking) │  │ (Peer Mgmt)  │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
└─────────────────────────────────────────────────────────┘
```

### Directory Structure
```
src/
├── main.cpp           # Entry point, user interface, mode selection
├── server.cpp         # Server that shares files with others
├── client.cpp         # Client that downloads files from others
├── peer.cpp           # Manages peer list and connections
├── file_utils.cpp     # File operations (split, merge, hash)
└── network_utils.cpp  # Low-level networking (sockets, send/receive)

include/
├── server.h           # Server class declaration
├── client.h           # Client class declaration
├── peer.h             # Peer structures and manager
├── file_utils.h       # File operation declarations
└── network_utils.h    # Network function declarations
```

---

## Source Files Explained

### 1. main.cpp - The Brain (Entry Point)

**Purpose:** Controls the entire application flow

**Key Responsibilities:**
- Parse command-line arguments
- Initialize network subsystem
- Choose operation mode (share/download/hybrid)
- Display interactive menu
- Coordinate server and client

**Main Functions:**
```cpp
int main()                    // Entry point
void runShareMode()           // Start sharing files
void runDownloadMode()        // Start downloading files
void runHybridMode()          // Interactive menu mode
bool parseArguments()         // Parse --mode=share, etc.
```

**Flow:**
```
1. User runs p2p_share.exe
2. main() initializes network
3. Parses arguments (or uses defaults)
4. Calls appropriate mode function
5. Cleans up and exits
```

---

### 2. server.cpp - The File Server

**Purpose:** Serves file chunks to other peers

**Key Responsibilities:**
- Listen for incoming connections
- Handle multiple clients simultaneously
- Send file chunks when requested
- Manage available chunks list
- Respond to ping requests

**Main Class:**
```cpp
class P2PServer {
    int port;                    // Port to listen on
    SOCKET serverSocket;         // Main server socket
    bool running;                // Is server active?
    vector<ChunkInfo> chunks;    // Available chunks
    string chunksDirectory;      // Where chunks are stored
}
```

**Key Functions:**
```cpp
bool start()                     // Start the server
void stop()                      // Stop the server
void run()                       // Main server loop
void handleClient()              // Handle one client
void processChunkRequest()       // Send a chunk
void processChunkListRequest()   // Send list of chunks
void loadChunksFromDirectory()   // Load available chunks
```

---

### 3. client.cpp - The File Downloader

**Purpose:** Downloads files from other peers

**Key Responsibilities:**
- Connect to peers
- Request chunk lists
- Download chunks in parallel
- Merge chunks into final file
- Track download progress

**Main Class:**
```cpp
class P2PClient {
    PeerManager peerManager;     // Manages peer connections
    string downloadDir;          // Where to save files
    string chunksDir;            // Temporary chunk storage
}
```

**Key Functions:**
```cpp
bool downloadFile()              // Download complete file
bool requestChunkFromPeer()      // Get one chunk
vector<ChunkInfo> requestChunkListFromPeer()  // Get chunk list
void assignChunksToPeers()       // Distribute chunk downloads
bool mergeDownloadedChunks()     // Combine chunks into file
```

---

### 4. peer.cpp - Peer Management

**Purpose:** Manages the list of peers and their chunks

**Key Responsibilities:**
- Load/save peer list from peers.txt
- Track which peers have which chunks
- Ping peers to check if online
- Update peer status

**Main Class:**
```cpp
class PeerManager {
    vector<Peer> peers;          // List of known peers
}
```

**Key Functions:**
```cpp
bool loadPeersFromFile()         // Load peers.txt
bool savePeersToFile()           // Save peers.txt
void addPeer()                   // Add new peer
void updatePeerChunks()          // Update chunk info
bool pingPeer()                  // Check if peer is online
```

---

### 5. file_utils.cpp - File Operations

**Purpose:** Handle all file-related operations

**Key Responsibilities:**
- Split files into chunks
- Merge chunks back into files
- Compute file hashes
- Verify file integrity
- Manage directories

**Key Functions:**
```cpp
vector<string> splitFile()       // Split file into chunks
bool mergeChunks()               // Merge chunks into file
string computeFileHash()         // Calculate file hash
bool verifyChunkIntegrity()      // Check chunk is valid
size_t getFileSize()             // Get file size
bool createDirectory()           // Create folder
```

---

### 6. network_utils.cpp - Networking Layer

**Purpose:** Low-level network operations

**Key Responsibilities:**
- Create and manage sockets
- Send and receive data
- Handle connections
- Platform-specific networking (Windows/Linux)

**Key Functions:**
```cpp
bool initializeNetwork()         // Initialize Winsock (Windows)
SOCKET createServerSocket()      // Create listening socket
SOCKET connectToPeer()           // Connect to peer
bool sendData()                  // Send bytes
bool receiveData()               // Receive bytes
bool sendChunk()                 // Send file chunk
bool receiveChunk()              // Receive file chunk
```

---

## Deep Dive: server.cpp

Let me explain `server.cpp` in detail - this is the heart of the sharing functionality.

### What server.cpp Does

**Analogy:** Think of it as a restaurant:
- **Server Socket** = Front door (people enter here)
- **Client Connections** = Tables (each customer gets a table)
- **Chunks** = Menu items (what you can serve)
- **handleClient()** = Waiter (serves each customer)

### Structure of server.cpp

```cpp
// 1. INCLUDES AND STRUCTURES
#include "server.h"
#include "peer.h"
#include "file_utils.h"
#include "network_utils.h"

struct ClientHandlerData {
    SOCKET clientSocket;
    P2PServer* server;
};

// 2. CONSTRUCTOR
P2PServer::P2PServer(int serverPort, const string& chunksDir) {
    port = serverPort;
    chunksDirectory = chunksDir;
    serverSocket = INVALID_SOCKET;
    running = false;
}

// 3. DESTRUCTOR
P2PServer::~P2PServer() {
    stop();  // Clean up when destroyed
}
```

### Key Function: start()

**What it does:** Starts the server and begins listening for connections

```cpp
bool P2PServer::start() {
    // Step 1: Load available chunks from disk
    loadChunksFromDirectory();
    
    // Step 2: Create server socket
    serverSocket = NetworkUtils::createServerSocket(port);
    if (serverSocket == INVALID_SOCKET) {
        return false;  // Failed to create socket
    }
    
    // Step 3: Mark as running
    running = true;
    
    // Step 4: Start server thread
    // This runs the main server loop in background
    #ifdef _WIN32
        serverThread = _beginthreadex(NULL, 0, serverThreadFunc, this, 0, NULL);
    #else
        pthread_create(&serverThread, NULL, serverThreadFunc, this);
    #endif
    
    return true;
}
```

**What happens:**
1. Scans `data/chunks/` folder for available chunks
2. Creates a socket and binds to port (e.g., 8080)
3. Starts listening for connections
4. Launches background thread to handle connections

### Key Function: run()

**What it does:** Main server loop - accepts connections forever

```cpp
void P2PServer::run() {
    cout << "Server listening for connections..." << endl;
    
    while (running) {
        // Wait for someone to connect
        SOCKET clientSocket = NetworkUtils::acceptConnection(serverSocket);
        
        if (clientSocket == INVALID_SOCKET) {
            continue;  // Connection failed, try again
        }
        
        // Someone connected! Create data for handler
        ClientHandlerData* data = new ClientHandlerData(clientSocket, this);
        
        // Start a new thread to handle this client
        #ifdef _WIN32
            HANDLE thread = _beginthreadex(NULL, 0, clientThreadFunc, data, 0, NULL);
            clientThreads.push_back(thread);
        #else
            pthread_t thread;
            pthread_create(&thread, NULL, clientThreadFunc, data);
            clientThreads.push_back(thread);
        #endif
    }
}
```

**What happens:**
1. Waits for a peer to connect
2. When connection arrives, accepts it
3. Creates a new thread to handle that peer
4. Goes back to waiting for more connections
5. Can handle MANY peers simultaneously!

### Key Function: handleClient()

**What it does:** Handles ONE client's request

```cpp
void P2PServer::handleClient(SOCKET clientSocket) {
    // Step 1: Receive the request message
    MessageHeader header;
    vector<char> data;
    
    if (!NetworkUtils::receiveMessage(clientSocket, header, data)) {
        NetworkUtils::closeSocket(clientSocket);
        return;  // Failed to receive
    }
    
    // Step 2: What does the client want?
    switch (header.type) {
        case MSG_CHUNK_REQUEST:
            // Client wants a specific chunk
            processChunkRequest(clientSocket, header);
            break;
            
        case MSG_CHUNK_LIST_REQUEST:
            // Client wants list of available chunks
            processChunkListRequest(clientSocket);
            break;
            
        case MSG_PEER_PING:
            // Client checking if we're alive
            processPingRequest(clientSocket);
            break;
            
        default:
            // Unknown request
            sendErrorResponse(clientSocket, "Unknown message type");
            break;
    }
    
    // Step 3: Close connection
    NetworkUtils::closeSocket(clientSocket);
}
```

**What happens:**
1. Receives a message from the peer
2. Checks what type of request it is
3. Calls appropriate handler function
4. Closes the connection

### Key Function: processChunkRequest()

**What it does:** Sends a specific chunk to a peer

```cpp
void P2PServer::processChunkRequest(SOCKET clientSocket, const MessageHeader& header) {
    // Step 1: Find the requested chunk
    ChunkInfo* requestedChunk = nullptr;
    for (auto& chunk : availableChunks) {
        if (chunk.filename == header.filename && 
            chunk.chunkIndex == header.chunkIndex) {
            requestedChunk = &chunk;
            break;
        }
    }
    
    // Step 2: Do we have it?
    if (requestedChunk == nullptr) {
        sendErrorResponse(clientSocket, "Chunk not found");
        return;
    }
    
    // Step 3: Get the chunk file path
    string chunkPath = FileUtils::getChunkPath(
        chunksDirectory, 
        requestedChunk->filename, 
        requestedChunk->chunkIndex
    );
    
    // Step 4: Does the file exist?
    if (!FileUtils::fileExists(chunkPath)) {
        sendErrorResponse(clientSocket, "Chunk file not found");
        return;
    }
    
    // Step 5: Send the chunk!
    NetworkUtils::sendChunk(clientSocket, chunkPath, *requestedChunk);
}
```

**What happens:**
1. Looks through available chunks for the requested one
2. Checks if we have it
3. Gets the file path for that chunk
4. Sends the chunk data over the network

### Key Function: processChunkListRequest()

**What it does:** Sends list of all available chunks

```cpp
void P2PServer::processChunkListRequest(SOCKET clientSocket) {
    // Simply send our list of available chunks
    NetworkUtils::sendChunkList(clientSocket, availableChunks);
}
```

**What happens:**
1. Takes the list of chunks we have
2. Sends it to the requesting peer
3. Peer now knows what files we can share

### Key Function: loadChunksFromDirectory()

**What it does:** Scans disk for available chunks

```cpp
void P2PServer::loadChunksFromDirectory() {
    // Step 1: Get all files in chunks directory
    vector<string> files = FileUtils::listFiles(chunksDirectory);
    availableChunks.clear();
    
    // Step 2: Look at each file
    for (const auto& file : files) {
        string filename = extractFilename(file);
        
        // Step 3: Is it a chunk file? (contains ".chunk.")
        if (filename.find(".chunk.") != string::npos) {
            // Parse: "document.pdf.chunk.0001"
            size_t chunkPos = filename.find(".chunk.");
            string originalName = filename.substr(0, chunkPos);
            string indexStr = filename.substr(chunkPos + 7);
            
            int chunkIndex = stoi(indexStr);
            size_t chunkSize = FileUtils::getFileSize(file);
            
            // Step 4: Create chunk info
            ChunkInfo chunk(originalName, chunkIndex, chunkSize);
            chunk.checksum = FileUtils::computeChunkHash(file);
            chunk.isAvailable = true;
            
            // Step 5: Add to our list
            availableChunks.push_back(chunk);
        }
    }
}
```

**What happens:**
1. Lists all files in `data/chunks/`
2. Finds files with `.chunk.` in the name
3. Parses filename to get original name and chunk number
4. Creates ChunkInfo for each chunk
5. Stores in availableChunks list

### Threading in server.cpp

**Why threads?** To handle multiple clients at once!

```
Without threads:
Client A connects → Server handles A → Client A disconnects
                                     ↓
                    Client B connects → Server handles B
                    (Client B had to WAIT!)

With threads:
Client A connects → Thread 1 handles A
Client B connects → Thread 2 handles B  (SIMULTANEOUS!)
Client C connects → Thread 3 handles C
```

**Thread Functions:**

```cpp
// Server thread - runs the main loop
static unsigned __stdcall serverThreadFunc(void* param) {
    P2PServer* server = static_cast<P2PServer*>(param);
    server->run();  // Start accepting connections
    return 0;
}

// Client thread - handles one client
static unsigned __stdcall clientThreadFunc(void* param) {
    ClientHandlerData* data = static_cast<ClientHandlerData*>(param);
    data->server->handleClient(data->clientSocket);
    delete data;  // Clean up
    return 0;
}
```

### Complete Flow Example

**Scenario:** Laptop B wants chunk 0 of "document.pdf" from Laptop A

```
LAPTOP A (Server):
1. start() called
   ↓
2. loadChunksFromDirectory()
   - Finds: document.pdf.chunk.0000
   - Adds to availableChunks
   ↓
3. run() starts
   - Waiting for connections...
   ↓
4. Laptop B connects!
   - acceptConnection() returns socket
   ↓
5. New thread created
   - Calls handleClient(socket)
   ↓
6. handleClient() receives message
   - Type: MSG_CHUNK_REQUEST
   - Filename: "document.pdf"
   - ChunkIndex: 0
   ↓
7. processChunkRequest() called
   - Finds chunk in availableChunks
   - Gets path: "data/chunks/document.pdf.chunk.0000"
   - Calls NetworkUtils::sendChunk()
   ↓
8. Chunk sent!
   - Thread closes socket
   - Thread exits
   ↓
9. Server continues waiting for more connections...
```

---

## How Files Work Together

### Example: Sharing a File

```
USER ACTION: Share document.pdf

main.cpp:
  ↓ runShareMode()
  ↓ Creates P2PServer(8080)
  
server.cpp:
  ↓ start()
  ↓ loadChunksFromDirectory()
  
file_utils.cpp:
  ↓ splitFile("document.pdf")
  ↓ Creates: document.pdf.chunk.0000
  ↓          document.pdf.chunk.0001
  ↓          document.pdf.chunk.0002
  
server.cpp:
  ↓ run() - waiting for connections
  
network_utils.cpp:
  ↓ createServerSocket(8080)
  ↓ acceptConnection() - waiting...
```

### Example: Downloading a File

```
USER ACTION: Download document.pdf

main.cpp:
  ↓ runDownloadMode()
  ↓ Creates P2PClient()
  
client.cpp:
  ↓ downloadFile("document.pdf")
  ↓ findPeersWithFile()
  
peer.cpp:
  ↓ loadPeersFromFile("peers.txt")
  ↓ Returns: [192.168.1.100:8080]
  
client.cpp:
  ↓ requestChunkListFromPeer()
  
network_utils.cpp:
  ↓ connectToPeer(192.168.1.100, 8080)
  ↓ sendMessage(MSG_CHUNK_LIST_REQUEST)
  
[SERVER SIDE - server.cpp]
  ↓ handleClient()
  ↓ processChunkListRequest()
  ↓ sendChunkList()
  
[CLIENT SIDE - client.cpp]
  ↓ receiveChunkList()
  ↓ assignChunksToPeers()
  ↓ downloadChunksParallel()
  
network_utils.cpp:
  ↓ sendChunkRequest(chunk 0)
  ↓ receiveChunk() → saves to disk
  ↓ sendChunkRequest(chunk 1)
  ↓ receiveChunk() → saves to disk
  
client.cpp:
  ↓ mergeDownloadedChunks()
  
file_utils.cpp:
  ↓ mergeChunks()
  ↓ Creates: data/downloads/document.pdf
  
DONE!
```

---

## Data Flow Examples

### Message Flow: Chunk Request

```
CLIENT                          SERVER
  |                               |
  |------ Connect to port ------->|
  |                               |
  |------ MSG_CHUNK_REQUEST ----->|
  |       (filename, index)       |
  |                               |
  |                          [Find chunk]
  |                          [Open file]
  |                               |
  |<----- MSG_CHUNK_RESPONSE -----|
  |       (chunk data)            |
  |                               |
  |------ Close connection ------>|
  |                               |
```

### File Splitting Flow

```
Original File: document.pdf (3.5 MB)
         ↓
   splitFile()
         ↓
    ┌────┴────┬────────┬────────┐
    ↓         ↓        ↓        ↓
Chunk 0   Chunk 1  Chunk 2  Chunk 3
(1 MB)    (1 MB)   (1 MB)   (0.5 MB)
    ↓         ↓        ↓        ↓
.chunk.   .chunk.  .chunk.  .chunk.
 0000      0001     0002     0003
```

### File Merging Flow

```
Downloaded Chunks:
.chunk.0000 (1 MB)
.chunk.0001 (1 MB)
.chunk.0002 (1 MB)
.chunk.0003 (0.5 MB)
         ↓
   mergeChunks()
         ↓
    [Sort by index]
         ↓
    [Read chunk 0]
    [Append to output]
    [Read chunk 1]
    [Append to output]
    [Read chunk 2]
    [Append to output]
    [Read chunk 3]
    [Append to output]
         ↓
document.pdf (3.5 MB)
```

---

## Summary

### The 6 Core Files:

1. **main.cpp** - The conductor, orchestrates everything
2. **server.cpp** - The waiter, serves files to others
3. **client.cpp** - The customer, requests files from others
4. **peer.cpp** - The phonebook, tracks who has what
5. **file_utils.cpp** - The chef, prepares and combines files
6. **network_utils.cpp** - The delivery service, moves data

### How They Collaborate:

```
main.cpp says: "User wants to share a file"
    ↓
server.cpp says: "I'll handle that"
    ↓
file_utils.cpp says: "Let me split it first"
    ↓
server.cpp says: "Now I'm ready to serve"
    ↓
network_utils.cpp says: "I'll handle the connections"
    ↓
peer.cpp says: "Here's who to talk to"
    ↓
client.cpp says: "I'll download from them"
    ↓
file_utils.cpp says: "Let me merge the pieces"
    ↓
main.cpp says: "Done! File transferred!"
```

**That's how the entire system works together!** 🎉
