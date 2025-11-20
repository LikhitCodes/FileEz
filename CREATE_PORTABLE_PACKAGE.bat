@echo off
echo ================================================
echo   Creating Portable P2P File Sharing Package
echo ================================================
echo.

REM Create package directory
set PACKAGE_DIR=P2P_FileShare_Portable
if exist %PACKAGE_DIR% rmdir /s /q %PACKAGE_DIR%
mkdir %PACKAGE_DIR%

echo [1/6] Creating directory structure...
mkdir %PACKAGE_DIR%\data
mkdir %PACKAGE_DIR%\data\shared
mkdir %PACKAGE_DIR%\data\chunks
mkdir %PACKAGE_DIR%\data\downloads

echo [2/6] Copying executable and DLLs...
copy p2p_share.exe %PACKAGE_DIR%\ >nul
if not exist %PACKAGE_DIR%\p2p_share.exe (
    echo ERROR: p2p_share.exe not found! Please compile first with: mingw32-make
    pause
    exit /b 1
)

REM Copy required DLLs if they exist
if exist libstdc++-6.dll (
    copy libstdc++-6.dll %PACKAGE_DIR%\ >nul
    echo    [OK] Included libstdc++-6.dll
)
if exist libgcc_s_dw2-1.dll (
    copy libgcc_s_dw2-1.dll %PACKAGE_DIR%\ >nul
    echo    [OK] Included libgcc_s_dw2-1.dll
)
if exist libgcc_s_seh-1.dll (
    copy libgcc_s_seh-1.dll %PACKAGE_DIR%\ >nul
    echo    [OK] Included libgcc_s_seh-1.dll
)
if exist libwinpthread-1.dll (
    copy libwinpthread-1.dll %PACKAGE_DIR%\ >nul
    echo    [OK] Included libwinpthread-1.dll
)

if not exist %PACKAGE_DIR%\libstdc++-6.dll (
    echo.
    echo    [WARNING] DLLs not found in current directory!
    echo    Run FIX_MISSING_DLLS.bat first to copy them.
    echo    Package may not work on computers without MinGW.
    echo.
)

echo [3/6] Copying configuration files...
echo # P2P File Sharing - Peer List > %PACKAGE_DIR%\peers.txt
echo # Format: IP:PORT >> %PACKAGE_DIR%\peers.txt
echo # Add peer addresses below: >> %PACKAGE_DIR%\peers.txt
echo # Example: 192.168.1.100:8080 >> %PACKAGE_DIR%\peers.txt
echo. >> %PACKAGE_DIR%\peers.txt

echo [4/6] Creating sample files...
echo This is a sample file for testing P2P file sharing. > %PACKAGE_DIR%\data\shared\sample.txt
echo You can share this file to test the system. >> %PACKAGE_DIR%\data\shared\sample.txt
echo Replace this with your own files to share. >> %PACKAGE_DIR%\data\shared\sample.txt

echo [5/6] Copying documentation...
copy USER_GUIDE.md %PACKAGE_DIR%\ >nul 2>&1
copy HOW_TO_USE.txt %PACKAGE_DIR%\ >nul 2>&1
copy TROUBLESHOOTING.md %PACKAGE_DIR%\ >nul 2>&1

echo [6/6] Creating launcher scripts...

REM Create Share Mode Launcher
echo @echo off > %PACKAGE_DIR%\SHARE_FILE.bat
echo echo ============================================ >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo   P2P File Share - SHARE MODE >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo ============================================ >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo. >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo Put your files in: data\shared\ >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo. >> %PACKAGE_DIR%\SHARE_FILE.bat
echo set /p filename="Enter filename to share: " >> %PACKAGE_DIR%\SHARE_FILE.bat
echo set /p port="Enter port (default 8080): " >> %PACKAGE_DIR%\SHARE_FILE.bat
echo if "%%port%%"=="" set port=8080 >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo. >> %PACKAGE_DIR%\SHARE_FILE.bat
echo echo Starting server on port %%port%%... >> %PACKAGE_DIR%\SHARE_FILE.bat
echo p2p_share.exe --mode=share --file=%%filename%% --port=%%port%% >> %PACKAGE_DIR%\SHARE_FILE.bat
echo pause >> %PACKAGE_DIR%\SHARE_FILE.bat

REM Create Download Mode Launcher
echo @echo off > %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo ============================================ >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo   P2P File Share - DOWNLOAD MODE >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo ============================================ >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo. >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo First, edit peers.txt to add peer addresses! >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo. >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo set /p filename="Enter filename to download: " >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo. >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo Downloading... >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo p2p_share.exe --mode=download --file=%%filename%% >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo. >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo echo File saved to: data\downloads\%%filename%% >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat
echo pause >> %PACKAGE_DIR%\DOWNLOAD_FILE.bat

REM Create Interactive Mode Launcher
echo @echo off > %PACKAGE_DIR%\START_P2P.bat
echo echo ============================================ >> %PACKAGE_DIR%\START_P2P.bat
echo echo   P2P File Share - INTERACTIVE MODE >> %PACKAGE_DIR%\START_P2P.bat
echo echo ============================================ >> %PACKAGE_DIR%\START_P2P.bat
echo echo. >> %PACKAGE_DIR%\START_P2P.bat
echo echo Your IP Address: >> %PACKAGE_DIR%\START_P2P.bat
echo for /f "tokens=2 delims=:" %%%%a in ('ipconfig ^| findstr /c:"IPv4 Address"') do echo %%%%a >> %PACKAGE_DIR%\START_P2P.bat
echo echo. >> %PACKAGE_DIR%\START_P2P.bat
echo set /p port="Enter port (default 8080): " >> %PACKAGE_DIR%\START_P2P.bat
echo if "%%port%%"=="" set port=8080 >> %PACKAGE_DIR%\START_P2P.bat
echo echo. >> %PACKAGE_DIR%\START_P2P.bat
echo p2p_share.exe --mode=hybrid --port=%%port%% >> %PACKAGE_DIR%\START_P2P.bat

REM Create README
echo ================================================ > %PACKAGE_DIR%\README.txt
echo   P2P FILE SHARING - PORTABLE VERSION >> %PACKAGE_DIR%\README.txt
echo ================================================ >> %PACKAGE_DIR%\README.txt
echo. >> %PACKAGE_DIR%\README.txt
echo QUICK START: >> %PACKAGE_DIR%\README.txt
echo. >> %PACKAGE_DIR%\README.txt
echo 1. Double-click START_P2P.bat for interactive mode >> %PACKAGE_DIR%\README.txt
echo    OR >> %PACKAGE_DIR%\README.txt
echo 2. Use SHARE_FILE.bat to share files >> %PACKAGE_DIR%\README.txt
echo 3. Use DOWNLOAD_FILE.bat to download files >> %PACKAGE_DIR%\README.txt
echo. >> %PACKAGE_DIR%\README.txt
echo IMPORTANT: >> %PACKAGE_DIR%\README.txt
echo - Both computers must be on the same network >> %PACKAGE_DIR%\README.txt
echo - Edit peers.txt to add other computer's IP:PORT >> %PACKAGE_DIR%\README.txt
echo - Put files to share in: data\shared\ >> %PACKAGE_DIR%\README.txt
echo - Downloaded files appear in: data\downloads\ >> %PACKAGE_DIR%\README.txt
echo. >> %PACKAGE_DIR%\README.txt
echo See USER_GUIDE.md for detailed instructions >> %PACKAGE_DIR%\README.txt
echo ================================================ >> %PACKAGE_DIR%\README.txt

echo.
echo ================================================
echo   Package Created Successfully!
echo ================================================
echo.
echo Package location: %PACKAGE_DIR%\
echo.
echo Contents:
echo   - p2p_share.exe (main application)
echo   - START_P2P.bat (interactive mode)
echo   - SHARE_FILE.bat (share mode)
echo   - DOWNLOAD_FILE.bat (download mode)
echo   - peers.txt (peer configuration)
echo   - data\ (directories for files)
echo   - Documentation files
echo.
echo To deploy:
echo   1. Copy entire '%PACKAGE_DIR%' folder to USB drive
echo   2. Copy to other laptop
echo   3. Run START_P2P.bat
echo.
echo Creating ZIP archive...
powershell -command "Compress-Archive -Path '%PACKAGE_DIR%' -DestinationPath 'P2P_FileShare_Portable.zip' -Force"
if exist P2P_FileShare_Portable.zip (
    echo.
    echo [OK] ZIP created: P2P_FileShare_Portable.zip
    echo     You can send this ZIP file to other laptops!
) else (
    echo.
    echo [INFO] Could not create ZIP. Copy the folder manually.
)
echo.
echo ================================================
pause
