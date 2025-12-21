@echo off
echo.
echo ========================================
echo      Building FileEz Desktop App
echo ========================================
echo.

REM Check if Node.js is available
node --version >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: Node.js is not installed or not in PATH
    echo Please install Node.js from https://nodejs.org/
    pause
    exit /b 1
)

echo Installing dependencies...
call npm install
if %errorlevel% neq 0 (
    echo ERROR: Failed to install dependencies
    pause
    exit /b 1
)

echo.
echo Building portable executable...
call npm run package-portable
if %errorlevel% neq 0 (
    echo ERROR: Failed to build portable executable
    pause
    exit /b 1
)

echo.
echo ========================================
echo        FileEz Build Complete!
echo ========================================
echo.
echo Your FileEz desktop app is ready at:
echo   portable\FileEz-win32-x64\FileEz.exe
echo.
echo To share with friends:
echo 1. Copy the entire "FileEz-win32-x64" folder
echo 2. Share it as a ZIP file
echo 3. Friends just run FileEz.exe - no installation needed!
echo.
echo The app includes:
echo   - FileEz.exe (main desktop app)
echo   - Backend server (p2p.exe)
echo   - Modern React dashboard
echo   - All dependencies bundled
echo.
pause