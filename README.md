# 🚀 P2P File Sharing System

A simple and secure peer-to-peer file sharing application that lets you share files directly between computers on the same network - no internet required!

## ✨ Features

- 🔒 **Secure Local Sharing** - Files stay on your network, never uploaded to the cloud
- 🌐 **Web Interface** - Easy-to-use browser-based dashboard
- ⚡ **Fast Transfer** - Direct computer-to-computer file transfer
- 📱 **Cross-Platform** - Works on Windows, with web interface accessible from any device
- 🔍 **File Discovery** - Automatically find and browse files from connected peers
- 📊 **Real-time Status** - See server status and available files instantly

## 🎯 Quick Start

### For Developers

1. **Clone and Build**
   ```bash
   git clone <repository-url>
   cd DecentralizedP2P
   cmake --build build
   ```

2. **Start the Backend**
   ```bash
   ./build/p2p.exe
   ```

3. **Start the Frontend** (in another terminal)
   ```bash
   cd frontend/react-dashboard-ui
   npm run dev
   ```

4. **Open Browser**
   - Go to `http://localhost:3000`
   - Click "Start Server" to begin sharing

### For Friends (Non-Developers)

1. **Get the Package**
   - Download `P2P_FileShare_Complete.zip`
   - Extract to your computer

2. **Start Sharing**
   - Double-click `START_P2P_SYSTEM.bat`
   - Web interface opens automatically
   - Follow the on-screen instructions

## 📖 How to Use

### 1. Setup
- Both users run the application on the same WiFi network
- Find your IP address using `FIND_MY_IP.bat`
- Add your friend's IP in the "Peer Management" section

### 2. Share Files
- Click "Choose File to Share" to upload files
- Files are automatically chunked and made available to peers
- Shared files appear in the `data/shared/` folder

### 3. Download Files
- Click "Browse & Download from Peers" to see available files
- Select files to download from connected peers
- Downloaded files appear in the `data/downloads/` folder

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐
│   Web Frontend  │    │   C++ Backend   │
│   (React/HTML)  │◄──►│   HTTP Server   │
│   Port 3000     │    │   Port 8080     │
└─────────────────┘    └─────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │  P2P Network    │
                    │  File Chunks    │
                    │  Port 8081      │
                    └─────────────────┘
```

## 📁 Project Structure

```
DecentralizedP2P/
├── src/                    # C++ source files
│   ├── main.cpp           # Application entry point
│   ├── control_server.cpp # HTTP API server
│   ├── server.cpp         # P2P server implementation
│   ├── client.cpp         # P2P client implementation
│   └── ...
├── include/               # Header files
├── frontend/              # React web interface
│   └── react-dashboard-ui/
├── data/                  # File storage
│   ├── shared/           # Files to share
│   ├── downloads/        # Downloaded files
│   └── chunks/           # File chunks
├── build/                # Compiled executables
└── P2P_FileShare_Complete/ # Deployment package
```

## 🔧 Technical Details

- **Backend**: C++ with httplib for HTTP server and custom P2P protocol
- **Frontend**: React with TypeScript and Tailwind CSS
- **File Transfer**: Chunked file system for efficient P2P sharing
- **Network**: TCP sockets for peer communication
- **Security**: Local network only, no external connections

## 🚀 Deployment

### Create Deployment Package
```bash
./DEPLOY_FOR_FRIENDS_V2.bat
```

This creates a complete package with:
- Compiled executable
- Built-in web interface
- Easy-start scripts
- User documentation

### System Requirements
- Windows 10/11
- Same WiFi network for all users
- Web browser (Chrome, Firefox, Edge)

## 🛠️ Development

### Build Requirements
- CMake 3.10+
- MinGW-w64 or Visual Studio
- Node.js 18+ (for frontend development)

### API Endpoints
- `GET /status` - Server status
- `POST /server/start` - Start P2P server
- `POST /files/upload` - Upload file
- `GET /files/browse` - Browse peer files
- `POST /peers/add` - Add peer
- And more...

## 🐛 Troubleshooting

### Common Issues

**"Failed to fetch" errors**
- Make sure the C++ server is running
- Check if Windows Firewall is blocking the application

**Can't see friend's files**
- Verify both users clicked "Start Server"
- Check IP addresses are correct in peer list
- Ensure you're on the same WiFi network

**Web interface won't load**
- Try `http://127.0.0.1:8080` instead of localhost:3000
- Make sure the server executable is running

## 📄 License

This project is open source. Feel free to use, modify, and distribute.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

