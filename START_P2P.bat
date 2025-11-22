@echo off
title P2P File Sharing System
color 0A

echo.
echo ================================================
echo   P2P File Sharing System - Easy Launcher
echo ================================================
echo.

REM Check if data directories exist, create if not
if not exist "data\shared" mkdir data\shared
if not exist "data\downloads" mkdir data\downloads
if not exist "data\chunks" mkdir data\chunks

REM Run the program in interactive mode
p2p_share.exe

pause
