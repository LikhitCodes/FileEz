`# P2P File Sharing System - User Guide

## Quick Start: Sharing Files Between Two Laptops

### Prerequisites
1. Both laptops must be on the same network (LAN/WiFi)
2. Compile the project on both laptops: `mingw32-make` or `make`
3. Know the IP addresses of both laptops

### Finding Your IP Address

**Windows:**
```cmd
ipconfig
```
Look for "IPv4 Address" under your active network adapter (e.g., 192.168.1.100)

**Linux/Mac:**
```bash
ifconfig
# or
ip addr show
```

---

## Scenario 1: Simple File Sharing (One-Way)

### Laptop A (Sender/Server) - Sharing a file

1. **Place your file in the shared directory:**
   ```
   Copy your file to: data/shared/
   Example: data/shared/document.pdf
   ```

2. **Start sharing the file:**
   ```cmd
   p2p_share.exe --mode=share --file=document.pdf --port=8080
   ```

3. **The system will:**
   - Split the file into chunks
   - Start a server on port 8080
   - Display: "Server is running. Press Enter to stop..."

4. **Note your IP address** (e.g., 192.168.1.100)

### Laptop B (Receiver/Client) - Downloading the file

1. **Edit peers.txt to add Laptop A:**
   ```
   192.168.1.100:8080
   ```

2. **Download the file:**
   ```cmd
   p2p_share.exe --mode=download --file=document.pdf
   ```

3. **The system will:**
   - Connect to Laptop A
   - Download chunks in parallel
   - Merge chunks into the final file
   - Save to: `data/downloads/document.pdf`

---

## Scenario 2: Interactive Mode (Recommended)

### Both Laptops - Run in Hybrid Mode

1. **On Laptop A (IP: 192.168.1.100):**
   ```cmd
   p2p_share.exe --mode=hybrid --port=8080
   ```

2. **On Laptop B (IP: 192.168.1.101):**
   ```cmd
   p2p_share.exe --mode=hybrid --port=8081
   ```

3. **Configure peers.txt on both laptops:**
   
   **Laptop A's peers.txt:**
   ```
   192.168.1.101:8081
   ```
   
   **Laptop B's peers.txt:**
   ```
   192.168.1.100:8080
   ```

4. **Use the interactive menu:**
   ```
   === P2P File Sharing Menu ===
   1. Share a file
   2. Download a file
   3. List available files
   4. Show server status
   5. Exit
   ```

### To Share a File:
- Choose option `1`
- Enter filename (must be in `data/shared/`)
- File is automatically split and shared

### To Download a File:
- Choose option `2`
- Enter filename
- File downloads automatically to `data/downloads/`

---

## Scenario 3: Multiple Peers (3+ Laptops)

### Setup Network

**Laptop A (192.168.1.100:8080)** - Has file1.pdf
**Laptop B (192.168.1.101:8081)** - Has file2.pdf  
**Laptop C (192.168.1.102:8082)** - Wants both files

### Configure peers.txt on Laptop C:
```
192.168.1.100:8080
192.168.1.101:8081
```

### Laptop C can now download from both:
```cmd
p2p_share.exe --mode=hybrid --port=8082
```

Then use menu option 2 to download files from either peer!

---

## Directory Structure

```
P2PFileShare/
├── data/
│   ├── shared/          # Put files here to share
│   ├── chunks/          # Temporary chunk storage (auto-created)
│   └── downloads/       # Downloaded files appear here
├── peers.txt            # List of peer IP:PORT addresses
└── p2p_share.exe        # The application
```

---

## Common Use Cases

### 1. Share a Large File
```cmd
# Laptop A (Sender)
p2p_share.exe --mode=share --file=movie.mp4 --port=8080

# Laptop B (Receiver)
p2p_share.exe --mode=download --file=movie.mp4
```

### 2. Share Multiple Files
```cmd
# Use hybrid mode for interactive sharing
p2p_share.exe --mode=hybrid --port=8080
```
Then use menu to share/download multiple files

### 3. Resume Downloads
If download fails, simply run the download command again. The system will re-download missing chunks.

---

## Troubleshooting

### "Failed to connect to peer"
- **Check firewall:** Allow p2p_share.exe through Windows Firewall
- **Verify IP address:** Use `ipconfig` to confirm
- **Check network:** Both laptops must be on same network
- **Test connection:** `ping 192.168.1.100` from other laptop

### "Port already in use"
- Choose a different port: `--port=8081`
- Or close other application using that port

### "File not found"
- Ensure file is in `data/shared/` directory
- Use exact filename (case-sensitive on Linux)

### "No peers found with file"
- Verify peers.txt has correct IP:PORT
- Ensure sender's server is running
- Check sender has the file in chunks directory

---

## Advanced Configuration

### Custom Directories
Edit the code or use default structure:
- Shared files: `data/shared/`
- Chunks: `data/chunks/`
- Downloads: `data/downloads/`

### Custom Chunk Size
Default: 1MB chunks (defined in `peer.h`)
```cpp
const int DEFAULT_CHUNK_SIZE = 1024 * 1024; // 1MB
```

### Firewall Configuration
**Windows Firewall:**
```cmd
# Allow inbound connections
netsh advfirewall firewall add rule name="P2P File Share" dir=in action=allow program="C:\path\to\p2p_share.exe" enable=yes
```

---

## Step-by-Step Example: Share a Document

### Laptop A (192.168.1.50) - Sharing

1. Copy file to share:
   ```cmd
   copy "C:\Users\John\Documents\report.pdf" data\shared\
   ```

2. Start sharing:
   ```cmd
   p2p_share.exe --mode=share --file=report.pdf --port=8080
   ```

3. Output:
   ```
   Sharing file: data/shared/report.pdf
   Splitting file into chunks...
   File split into 5 chunks
   Starting P2P server on port 8080...
   Server is running. Press Enter to stop...
   ```

### Laptop B (192.168.1.51) - Downloading

1. Edit peers.txt:
   ```
   192.168.1.50:8080
   ```

2. Download file:
   ```cmd
   p2p_share.exe --mode=download --file=report.pdf
   ```

3. Output:
   ```
   Starting download for file: report.pdf
   Found 1 peers with the file
   File has 5 chunks
   Downloading 5 chunks...
   Download progress: 5/5 (100%)
   Download complete: data/downloads/report.pdf
   ```

4. Your file is now at: `data\downloads\report.pdf`

---

## Performance Tips

1. **Larger files = Better performance** (chunking overhead is amortized)
2. **More peers = Faster downloads** (parallel chunk downloads)
3. **Same network = Best speed** (LAN is faster than WiFi)
4. **Keep server running** while others download

---

## Security Notes

⚠️ **Important:** This is a demonstration project for educational purposes.

- **No encryption:** Files are transferred in plain text
- **No authentication:** Anyone on network can connect
- **LAN only:** Designed for trusted local networks
- **Not for production:** Use for learning/testing only

For production use, consider:
- Adding TLS/SSL encryption
- Implementing user authentication
- Adding access control lists
- Using VPN for remote connections

---

## Command Reference

### Share Mode
```cmd
p2p_share.exe --mode=share --file=<filename> --port=<port>
```

### Download Mode
```cmd
p2p_share.exe --mode=download --file=<filename>
```

### Hybrid Mode (Interactive)
```cmd
p2p_share.exe --mode=hybrid --port=<port>
```

### Options
- `--mode=<share|download|hybrid>` - Operation mode
- `--file=<filename>` - File to share/download
- `--port=<port>` - Server port (default: 8080)
- `--peers=<file>` - Peers file (default: peers.txt)
- `--verbose` - Verbose output
- `--help` - Show help

---

## FAQ

**Q: Can I share files over the internet?**
A: Not directly. This is designed for LAN. You'd need port forwarding and know external IPs.

**Q: How many peers can I connect to?**
A: Up to 100 peers (configurable in peer.h)

**Q: What file types are supported?**
A: All file types (binary safe)

**Q: Can I pause/resume downloads?**
A: Not currently, but you can restart and it will re-download

**Q: Is it faster than USB transfer?**
A: Depends on network speed. Gigabit LAN ≈ USB 2.0 speed

**Q: Can I share folders?**
A: Not directly. Share files individually or zip the folder first.

---

## Getting Help

If you encounter issues:
1. Check this guide's Troubleshooting section
2. Verify network connectivity with `ping`
3. Check firewall settings
4. Ensure both laptops have the same version
5. Try with a small test file first

---

## Example Session

```
Laptop A (Server):
> p2p_share.exe --mode=share --file=presentation.pptx --port=8080
Sharing file: data/shared/presentation.pptx
File split into 3 chunks
Server is running on port 8080...

Laptop B (Client):
> p2p_share.exe --mode=download --file=presentation.pptx
Connecting to peers...
Found 1 peers with the file
Downloading 3 chunks...
[████████████████████] 100%
Download complete!
File saved to: data/downloads/presentation.pptx
```

---

**Congratulations!** You're now ready to share files peer-to-peer! 🎉
