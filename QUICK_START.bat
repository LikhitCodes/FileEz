@echo off
echo ================================================
echo   P2P File Sharing - Quick Start Setup
echo ================================================
echo.

REM Create necessary directories
if not exist "data\shared" mkdir data\shared
if not exist "data\chunks" mkdir data\chunks
if not exist "data\downloads" mkdir data\downloads

echo [OK] Directories created
echo.

REM Get local IP address
echo Your IP Address:
for /f "tokens=2 delims=:" %%a in ('ipconfig ^| findstr /c:"IPv4 Address"') do echo %%a
echo.

REM Check if peers.txt exists
if not exist "peers.txt" (
    echo Creating default peers.txt...
    echo # P2P File Sharing - Peer List > peers.txt
    echo # Format: IP:PORT >> peers.txt
    echo # Example: >> peers.txt
    echo # 192.168.1.100:8080 >> peers.txt
    echo # 192.168.1.101:8081 >> peers.txt
    echo. >> peers.txt
    echo [OK] Created peers.txt - Edit this file to add peer addresses
) else (
    echo [OK] peers.txt already exists
)
echo.

REM Create a test file
if not exist "data\shared\test.txt" (
    echo Creating test file...
    echo This is a test file for P2P file sharing. > data\shared\test.txt
    echo You can share this file to test the system. >> data\shared\test.txt
    echo [OK] Created data\shared\test.txt
)
echo.

echo ================================================
echo   Setup Complete!
echo ================================================
echo.
echo Next Steps:
echo.
echo 1. Edit peers.txt and add other laptop's IP:PORT
echo    Example: 192.168.1.100:8080
echo.
echo 2. To SHARE a file:
echo    p2p_share.exe --mode=share --file=test.txt --port=8080
echo.
echo 3. To DOWNLOAD a file (on other laptop):
echo    p2p_share.exe --mode=download --file=test.txt
echo.
echo 4. For interactive mode:
echo    p2p_share.exe --mode=hybrid --port=8080
echo.
echo 5. For help:
echo    p2p_share.exe --help
echo.
echo See USER_GUIDE.md for detailed instructions
echo ================================================
pause
