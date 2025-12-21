# 🚀 FileEz - P2P File Sharing Desktop App

**Complete, sharable desktop application for secure peer-to-peer file sharing**

## ✨ What is FileEz?

FileEz is an all-in-one desktop app that bundles everything needed for P2P file sharing:
- ✅ Backend server (C++ executable)
- ✅ Modern web interface (React dashboard)
- ✅ Electron desktop wrapper
- ✅ All dependencies included

## 🎯 Quick Start

1. **Run FileEz.exe** (the main executable)
2. **Find your IP** using the network info in the app
3. **Add friends** as peers using their IP addresses
4. **Start sharing** files instantly!

## 📁 What's Included

```
FileEz/
├── FileEz.exe              # Main desktop app (double-click to start)
├── p2p.exe                 # Backend server (runs automatically)
├── web/                    # Modern React dashboard
├── data/                   # File storage (shared, downloads, chunks)
├── peers.txt               # Your peer connections
└── node_modules/           # Electron runtime (required)
```

## 🔧 Features

- **🔒 Secure**: Files stay on your local network
- **⚡ Fast**: Direct peer-to-peer transfer
- **📊 Modern UI**: Beautiful React dashboard with transfer history
- **🎯 Complete**: Everything bundled, no separate installs needed
- **📱 Cross-Platform**: Web interface works on any device

## 🌐 Network Setup

- **Same Network**: All users on same WiFi/LAN
- **Ports**: 8080 (web interface), 8081 (P2P transfers)
- **Firewall**: Allow FileEz through Windows Firewall

## 🚀 Building FileEz

To create the executable:

```bash
# Install dependencies (first time only)
npm install

# Create portable executable
npm run package-portable

# Create installer
npm run build
```

## 📦 Sharing FileEz

The entire `FileEz` folder is portable and sharable:
1. Zip the complete `FileEz` folder
2. Share with friends
3. They extract and run `FileEz.exe`
4. No additional setup required!

---

**FileEz - Making P2P file sharing simple and accessible! 🎉**