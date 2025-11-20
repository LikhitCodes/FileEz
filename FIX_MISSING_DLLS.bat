@echo off
echo ================================================
echo   Finding and Copying Required DLLs
echo ================================================
echo.

REM Find MinGW bin directory
set MINGW_BIN=C:\MinGW\bin
if not exist "%MINGW_BIN%" set MINGW_BIN=C:\mingw\bin
if not exist "%MINGW_BIN%" set MINGW_BIN=C:\mingw32\bin
if not exist "%MINGW_BIN%" set MINGW_BIN=C:\Program Files\mingw-w64\bin
if not exist "%MINGW_BIN%" set MINGW_BIN=C:\msys64\mingw64\bin

echo Searching for MinGW DLLs...
echo.

REM Check if we found MinGW
if not exist "%MINGW_BIN%" (
    echo ERROR: Could not find MinGW bin directory!
    echo.
    echo Please locate your MinGW installation and copy these files manually:
    echo   - libstdc++-6.dll
    echo   - libgcc_s_dw2-1.dll  OR  libgcc_s_seh-1.dll
    echo   - libwinpthread-1.dll
    echo.
    echo Common locations:
    echo   C:\MinGW\bin\
    echo   C:\msys64\mingw64\bin\
    echo.
    pause
    exit /b 1
)

echo Found MinGW at: %MINGW_BIN%
echo.

REM Copy required DLLs to current directory
echo Copying DLLs...

if exist "%MINGW_BIN%\libstdc++-6.dll" (
    copy "%MINGW_BIN%\libstdc++-6.dll" . >nul
    echo [OK] libstdc++-6.dll
) else (
    echo [MISSING] libstdc++-6.dll
)

if exist "%MINGW_BIN%\libgcc_s_dw2-1.dll" (
    copy "%MINGW_BIN%\libgcc_s_dw2-1.dll" . >nul
    echo [OK] libgcc_s_dw2-1.dll
) else if exist "%MINGW_BIN%\libgcc_s_seh-1.dll" (
    copy "%MINGW_BIN%\libgcc_s_seh-1.dll" . >nul
    echo [OK] libgcc_s_seh-1.dll
) else (
    echo [MISSING] libgcc_s_dw2-1.dll or libgcc_s_seh-1.dll
)

if exist "%MINGW_BIN%\libwinpthread-1.dll" (
    copy "%MINGW_BIN%\libwinpthread-1.dll" . >nul
    echo [OK] libwinpthread-1.dll
) else (
    echo [INFO] libwinpthread-1.dll not found (may not be needed)
)

echo.
echo ================================================
echo   DLLs copied successfully!
echo ================================================
echo.
echo Now run: CREATE_PORTABLE_PACKAGE.bat
echo.
echo The package will include these DLLs and work
echo on any Windows computer without MinGW installed.
echo.
pause
