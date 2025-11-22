# P2P File Sharing - Quick Start Guide

## For Your Friend (First Time User)

### Step 1: Extract and Run
1. Extract the portable package to any folder
2. Double-click `START_P2P.bat`

### Step 2: Share a File
1. Copy the file you want to share into the `data/shared` folder
2. In the menu, select option `1` (Share a file)
3. Enter the filename (e.g., `movie.mp4`)
4. Press Enter to use default port (8080) or enter a custom port
5. **Share your IP address and port with friends!**
   - Your IP will be displayed on screen
   - Example: `192.168.1.100:8080`

### Step 3: Keep the Program Running
- Leave the program running while others download
- Press Enter when you want to stop sharing

---

## For You (Downloading from Friend)

### Step 1: Add Your Friend's IP
**Option A - Automatic Discovery (Same Network):**
1. Run `START_P2P.bat`
2. Select option `3` (Discover peers)
3. Wait for scan to complete
4. Type `y` to add discovered peers

**Option B - Manual Entry:**
1. Open `peers.txt` in a text editor
2. Add your friend's IP and port on a new line
   - Example: `192.168.1.100:8080`
3. Save the file

### Step 2: Browse Available Files (Optional)
1. Run `START_P2P.bat`
2. Select option `3` (Browse available files on peers)
3. See all files available on connected peers
4. Choose to download directly from the list

### Step 3: Download the File
1. Run `START_P2P.bat` (if not already running)
2. Select option `2` (Download a file)
3. Enter the filename your friend is sharing
4. Wait for download to complete
5. Find your file in `data/downloads` folder

---

## Command Line Usage (Advanced)

### Share a file:
```
p2p_share.exe --mode=share --file=document.pdf --port=8080
```

### Download a file:
```
p2p_share.exe --mode=download --file=document.pdf
```

### Discover peers:
```
p2p_share.exe --mode=discover --port=8080
```

---

## Troubleshooting

### "No peers found"
- Make sure your friend's computer is on the same network
- Check that firewall isn't blocking the port
- Verify the IP address and port are correct in `peers.txt`

### "File not found"
- Make sure the file is in the `data/shared` folder
- Check that the filename matches exactly (including extension)
- Verify your friend is still running the share mode

### Firewall Issues
- Windows may ask to allow network access - click "Allow"
- If blocked, add exception for `p2p_share.exe` in Windows Firewall

---

## Network Requirements

- **Same Local Network**: Both computers should be on the same WiFi/LAN
- **Port**: Default is 8080 (can be changed)
- **Firewall**: Must allow incoming connections on the chosen port

---

## Tips

1. **Larger Files**: The system automatically splits large files into chunks for efficient transfer
2. **Multiple Peers**: You can download from multiple peers simultaneously for faster speeds
3. **Keep Sharing**: Leave share mode running so others can download
4. **Check Peers**: Use option 4 in the menu to view your configured peers
