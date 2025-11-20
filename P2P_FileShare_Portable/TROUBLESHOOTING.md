# P2P File Sharing - Troubleshooting Guide

## Common Issues and Solutions

### 1. "Failed to connect to peer"

**Symptoms:**
```
Failed to connect to peer 192.168.1.100:8080
Error: Connection refused
```

**Solutions:**

✅ **Check if server is running on the peer:**
```bash
# On the peer machine, verify server is running
# You should see: "Server is running..."
```

✅ **Verify IP address:**
```bash
# Windows
ipconfig

# Linux/Mac
ifconfig
ip addr show
```

✅ **Test network connectivity:**
```bash
ping 192.168.1.100
```
If ping fails, you're not on the same network.

✅ **Check firewall:**
```bash
# Windows: Allow p2p_share.exe through firewall
# Control Panel → Windows Defender Firewall → Allow an app

# Or use command:
netsh advfirewall firewall add rule name="P2P Share" dir=in action=allow program="C:\path\to\p2p_share.exe"
```

✅ **Verify port number:**
- Ensure peers.txt has correct port (e.g., 8080)
- Server must be started with same port: `--port=8080`

---

### 2. "Port already in use"

**Symptoms:**
```
Failed to bind socket to port 8080
Error: Address already in use
```

**Solutions:**

✅ **Use a different port:**
```bash
p2p_share.exe --mode=share --file=test.txt --port=8081
```

✅ **Find what's using the port:**
```bash
# Windows
netstat -ano | findstr :8080

# Linux/Mac
lsof -i :8080
```

✅ **Kill the process using the port:**
```bash
# Windows (use PID from netstat)
taskkill /PID <process_id> /F

# Linux/Mac
kill -9 <process_id>
```

---

### 3. "File not found"

**Symptoms:**
```
Error: File not found: data/shared/document.pdf
```

**Solutions:**

✅ **Check file location:**
```bash
# File must be in data/shared/ directory
dir data\shared\        # Windows
ls data/shared/         # Linux/Mac
```

✅ **Check filename spelling:**
- Filenames are case-sensitive on Linux/Mac
- Use exact filename including extension

✅ **Use correct path:**
```bash
# Correct
p2p_share.exe --mode=share --file=document.pdf

# Wrong (don't include path)
p2p_share.exe --mode=share --file=data/shared/document.pdf
```

---

### 4. "No peers found with file"

**Symptoms:**
```
No peers found with file: document.pdf
```

**Solutions:**

✅ **Verify peers.txt:**
```
# peers.txt should contain:
192.168.1.100:8080
```

✅ **Check peer is sharing the file:**
```bash
# On peer machine, verify file was split into chunks
dir data\chunks\        # Should show .chunk.0000, .chunk.0001, etc.
```

✅ **Restart peer's server:**
```bash
# Stop and restart the server to reload chunks
p2p_share.exe --mode=share --file=document.pdf --port=8080
```

✅ **Check filename matches:**
- Sender and receiver must use exact same filename
- Case-sensitive on Linux/Mac

---

### 5. "Download failed" or "Failed to receive chunk"

**Symptoms:**
```
Failed to receive chunk 3
Download failed
```

**Solutions:**

✅ **Check network stability:**
```bash
# Test with continuous ping
ping -t 192.168.1.100    # Windows
ping 192.168.1.100       # Linux/Mac (Ctrl+C to stop)
```

✅ **Retry download:**
```bash
# Simply run download command again
p2p_share.exe --mode=download --file=document.pdf
```

✅ **Check disk space:**
```bash
# Ensure enough space in data/downloads/
dir data\downloads\     # Windows
df -h                   # Linux/Mac
```

✅ **Verify chunk files exist on server:**
```bash
# On server machine
dir data\chunks\document.pdf.chunk.*
```

---

### 6. "Merged file differs from original"

**Symptoms:**
```
ERROR: Merged file differs from original
File integrity check failed
```

**Solutions:**

✅ **Re-download the file:**
```bash
# Delete partial download
del data\downloads\document.pdf
del data\chunks_download\*

# Download again
p2p_share.exe --mode=download --file=document.pdf
```

✅ **Check network errors:**
- Unstable WiFi can corrupt chunks
- Use wired connection for large files

✅ **Verify source file:**
```bash
# On server, check original file is intact
# Compare file sizes
```

---

### 7. "Server not responding" or hangs

**Symptoms:**
- Server starts but doesn't respond to requests
- Application hangs or freezes

**Solutions:**

✅ **Restart the application:**
```bash
# Press Ctrl+C to stop
# Start again
p2p_share.exe --mode=share --file=test.txt --port=8080
```

✅ **Check for multiple instances:**
```bash
# Windows
tasklist | findstr p2p_share

# Kill extra instances
taskkill /IM p2p_share.exe /F
```

✅ **Try different port:**
```bash
p2p_share.exe --mode=share --file=test.txt --port=9000
```

---

### 8. Network Configuration Issues

**Both laptops must be on same network:**

✅ **Check network name (SSID):**
- Both must connect to same WiFi network
- Or both connected via Ethernet to same router

✅ **Check IP range:**
```
Good: 192.168.1.100 and 192.168.1.101 (same subnet)
Bad:  192.168.1.100 and 192.168.2.100 (different subnets)
```

✅ **Disable VPN:**
- VPN can route traffic differently
- Temporarily disable VPN for P2P sharing

✅ **Check router settings:**
- Some routers have "AP Isolation" enabled
- This prevents devices from talking to each other
- Disable AP Isolation in router settings

---

### 9. Compilation Errors

**"make: command not found"**
```bash
# Windows: Use mingw32-make instead
mingw32-make

# Or install make
```

**"g++: command not found"**
```bash
# Install MinGW (Windows) or GCC (Linux)
# Windows: Download from mingw.org
# Linux: sudo apt-get install g++
```

**"undefined reference" errors**
```bash
# Clean and rebuild
mingw32-make clean
mingw32-make
```

---

### 10. Performance Issues

**Slow download speed:**

✅ **Check network speed:**
```bash
# Test with large ping
ping -l 65000 192.168.1.100    # Windows
ping -s 65000 192.168.1.100    # Linux/Mac
```

✅ **Use wired connection:**
- Ethernet is faster than WiFi
- Gigabit Ethernet: ~125 MB/s
- WiFi: ~10-50 MB/s

✅ **Close other network applications:**
- Stop downloads, streaming, etc.
- Reduces network congestion

✅ **Add more peers:**
- Download from multiple peers simultaneously
- Each peer provides different chunks

---

## Diagnostic Commands

### Check if server is listening:
```bash
# Windows
netstat -an | findstr :8080

# Linux/Mac
netstat -an | grep :8080
lsof -i :8080
```

### Test connection manually:
```bash
# Try to connect to peer
telnet 192.168.1.100 8080

# If connection succeeds, server is reachable
# Press Ctrl+] then 'quit' to exit
```

### View network interfaces:
```bash
# Windows
ipconfig /all

# Linux/Mac
ifconfig -a
ip addr show
```

### Check routing:
```bash
# Windows
route print

# Linux/Mac
route -n
netstat -rn
```

---

## Still Having Issues?

### Debug Mode
Run with verbose output:
```bash
p2p_share.exe --mode=share --file=test.txt --verbose
```

### Test with Small File
```bash
# Create tiny test file
echo "test" > data\shared\tiny.txt

# Try sharing this first
p2p_share.exe --mode=share --file=tiny.txt --port=8080
```

### Verify Basic Functionality
```bash
# Run system tests
p2p_share.exe --test
```

### Check Logs
Look for error messages in the console output

---

## Prevention Tips

1. **Always use same network** - Connect both laptops to same WiFi/LAN
2. **Note IP addresses** - Write down IPs before starting
3. **Test with small files first** - Verify system works before large transfers
4. **Keep server running** - Don't close server while others download
5. **Use stable connection** - Wired > WiFi for reliability
6. **Check firewall first** - Allow application before troubleshooting
7. **Update peers.txt** - Keep peer list current

---

## Error Messages Reference

| Error Message | Meaning | Solution |
|--------------|---------|----------|
| "Connection refused" | Server not running | Start server on peer |
| "Connection timeout" | Network issue | Check firewall/network |
| "Address already in use" | Port occupied | Use different port |
| "File not found" | Missing file | Check data/shared/ |
| "Failed to bind socket" | Port unavailable | Change port number |
| "Invalid IP address" | Wrong IP format | Use format: 192.168.1.100 |
| "No peers found" | Empty peers.txt | Add peer addresses |
| "Chunk not found" | Missing chunk file | Re-share file on server |

---

## Getting More Help

If you've tried everything:

1. ✅ Verified both laptops on same network
2. ✅ Checked firewall settings
3. ✅ Tested with small file
4. ✅ Confirmed IP addresses correct
5. ✅ Tried different ports

And it still doesn't work, check:
- Router documentation for AP Isolation settings
- Network administrator if on corporate network
- Antivirus software blocking connections

---

**Remember:** This system is designed for LOCAL networks only. Both devices must be on the same LAN/WiFi network to communicate.
