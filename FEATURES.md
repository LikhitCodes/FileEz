# P2P File Sharing System - Features

## Core Features

### 1. **Interactive Mode** (NEW!)
- Simple menu-driven interface
- No command-line knowledge required
- Automatically detects and displays your IP address
- Just run `START_P2P.bat` and follow the menu

### 2. **File Sharing**
- Share any file from `data/shared/` folder
- Automatic file chunking for large files
- Server runs on configurable port (default: 8080)
- Shows your IP:PORT for others to connect

### 3. **File Downloading**
- Download files from configured peers
- Automatic chunk reassembly
- Checksum verification for integrity
- Multi-peer download support (faster speeds)

### 4. **Browse Available Files** (NEW!)
- See all files available on connected peers
- Shows which peers have each file
- No need to guess filenames
- Option to download directly from browse list

### 5. **Automatic Peer Discovery** (NEW!)
- Scans local network for active P2P servers
- Automatically finds peers on same subnet
- One-click to add discovered peers to configuration
- No manual IP entry needed

### 6. **Peer Management**
- View configured peers list
- Add peers manually via `peers.txt`
- Support for multiple peers
- Automatic peer status checking

---

## Technical Features

### File Handling
- **Chunking**: Large files split into manageable chunks (default: 1MB)
- **Metadata**: Chunk information stored for tracking
- **Verification**: SHA-256 checksums for data integrity
- **Reassembly**: Automatic merging of downloaded chunks

### Network
- **Protocol**: TCP/IP socket communication
- **Port Configuration**: Customizable port numbers
- **Timeout Handling**: Configurable connection timeouts
- **Multi-threading**: Parallel chunk downloads (future enhancement)

### Security
- **Local Network Only**: Designed for LAN use
- **Checksum Verification**: Ensures file integrity
- **No Authentication**: Trust-based system for local networks

---

## User Interface Options

### 1. Interactive Mode (Easiest)
```
Just run: START_P2P.bat
```
- Menu-driven interface
- Perfect for beginners
- All features accessible

### 2. Quick Launchers
```
SHARE_FILE.bat    - Quick share mode
DOWNLOAD_FILE.bat - Quick download mode
```
- Pre-configured scripts
- Minimal input required

### 3. Command Line (Advanced)
```bash
# Share a file
p2p_share.exe --mode=share --file=movie.mp4 --port=8080

# Download a file
p2p_share.exe --mode=download --file=movie.mp4

# Discover peers
p2p_share.exe --mode=discover --port=8080

# Browse files (via interactive mode)
p2p_share.exe
```

---

## Menu Options Explained

### Option 1: Share a File
1. Lists files in `data/shared/` folder
2. Enter filename to share
3. Choose port (or use default)
4. Server starts and displays your IP:PORT
5. Share this info with friends
6. Keep running while others download

### Option 2: Download a File
1. Enter filename to download
2. System connects to configured peers
3. Downloads chunks from available peers
4. Reassembles file automatically
5. Saves to `data/downloads/` folder

### Option 3: Browse Available Files
1. Connects to all configured peers
2. Requests file list from each peer
3. Shows all available files
4. Displays which peers have each file
5. Option to download immediately

### Option 4: Discover Peers on Network
1. Scans local network (192.168.x.x)
2. Tests each IP for P2P server
3. Shows discovered peers
4. Option to add to `peers.txt`
5. May take 1-2 minutes for full scan

### Option 5: View Peers List
1. Shows all configured peers from `peers.txt`
2. Displays IP:PORT for each peer
3. Quick reference for your network

### Option 6: Exit
- Cleanly closes the program
- Stops any running servers

---

## File Structure

```
P2P_FileShare_Portable/
├── p2p_share.exe              # Main executable
├── START_P2P.bat              # Interactive launcher
├── SHARE_FILE.bat             # Quick share
├── DOWNLOAD_FILE.bat          # Quick download
├── peers.txt                  # Peer configuration
│
├── data/
│   ├── shared/                # Put files to share here
│   ├── downloads/             # Downloaded files appear here
│   ├── chunks/                # Temporary chunk storage (server)
│   └── chunks_download/       # Temporary chunk storage (client)
│
└── Documentation/
    ├── QUICK_START.md         # Quick reference
    ├── DEPLOYMENT_GUIDE.md    # Detailed setup guide
    ├── FEATURES.md            # This file
    └── USER_GUIDE.md          # Complete user manual
```

---

## Workflow Examples

### Scenario 1: Friend Wants to Share a Movie

**Friend's Side:**
1. Copies `movie.mp4` to `data/shared/`
2. Runs `START_P2P.bat`
3. Selects "1. Share a file"
4. Enters "movie.mp4"
5. Presses Enter (uses default port 8080)
6. Tells you: "My IP is 192.168.1.100, port 8080"

**Your Side:**
1. Run `START_P2P.bat`
2. Select "4. Discover peers" (if on same network)
   - OR manually add `192.168.1.100:8080` to `peers.txt`
3. Select "3. Browse available files"
4. See "movie.mp4" in the list
5. Choose to download
6. File appears in `data/downloads/movie.mp4`

### Scenario 2: Multiple Friends Sharing Different Files

**Setup:**
- Friend A: Sharing `document.pdf` on 192.168.1.100:8080
- Friend B: Sharing `music.mp3` on 192.168.1.101:8080
- Friend C: Sharing `photo.jpg` on 192.168.1.102:8080

**Your Side:**
1. Run `START_P2P.bat`
2. Select "4. Discover peers"
3. All three friends discovered automatically
4. Select "3. Browse available files"
5. See all three files listed with their sources
6. Download any or all files

---

## Performance

### Speed Factors
- **Network Speed**: Limited by your LAN speed (typically 100Mbps-1Gbps)
- **Chunk Size**: Optimized at 1MB per chunk
- **Multiple Peers**: Can download different chunks from different peers
- **Disk Speed**: SSD vs HDD affects write speed

### Typical Performance
- **Small Files** (<10MB): Nearly instant
- **Medium Files** (10-100MB): Few seconds
- **Large Files** (>1GB): Depends on network, typically 1-10 minutes on gigabit LAN

---

## Limitations

### Network
- ❌ Only works on local network (LAN/WiFi)
- ❌ Cannot share over internet
- ❌ Both computers must be on same subnet
- ❌ Firewall must allow the port

### Files
- ✅ No file size limit (tested up to several GB)
- ✅ Any file type supported
- ❌ Filenames are case-sensitive
- ❌ Special characters in filenames may cause issues

### Security
- ❌ No encryption (files sent in plain text)
- ❌ No authentication (anyone on network can connect)
- ❌ No access control
- ✅ Checksum verification prevents corruption

---

## Future Enhancements (Potential)

- [ ] True parallel chunk downloads (multi-threading)
- [ ] Resume interrupted downloads
- [ ] File encryption for security
- [ ] User authentication
- [ ] Web-based interface
- [ ] Mobile app support
- [ ] Torrent-like protocol improvements
- [ ] Bandwidth throttling
- [ ] Upload/download statistics
- [ ] File search functionality

---

## Comparison with Other Solutions

### vs USB Drive
- ✅ No physical transfer needed
- ✅ Faster for large files
- ✅ Multiple people can download simultaneously
- ❌ Requires network connection

### vs Cloud Storage (Dropbox, Google Drive)
- ✅ No internet required
- ✅ No file size limits
- ✅ No upload/download from cloud
- ✅ Faster on local network
- ❌ Not accessible remotely

### vs Email
- ✅ No file size limits
- ✅ Much faster
- ✅ No email account needed
- ❌ Requires both computers online simultaneously

### vs FTP Server
- ✅ Easier to set up
- ✅ No server configuration
- ✅ User-friendly interface
- ❌ Less features than full FTP

---

## System Requirements

### Minimum
- Windows 7 or later (or Linux with modifications)
- 100MB free disk space
- Network adapter (WiFi or Ethernet)
- 512MB RAM

### Recommended
- Windows 10 or later
- 1GB free disk space
- Gigabit Ethernet or WiFi 5/6
- 2GB RAM

### Network Requirements
- Same local network (LAN/WiFi)
- Open port (default 8080)
- Firewall exception for p2p_share.exe

---

## Support & Troubleshooting

See `DEPLOYMENT_GUIDE.md` for detailed troubleshooting steps.

Common issues:
1. **"No peers found"** → Check network and peers.txt
2. **"Connection refused"** → Check firewall
3. **"File not found"** → Verify filename and peer is sharing

---

## Credits

Built with:
- C++17
- WinSock2 (Windows networking)
- MinGW compiler
- Standard Template Library (STL)

---

**Version**: 1.0  
**Last Updated**: 2024  
**License**: Educational/Personal Use
