@echo off 
title P2P File Sharing System 
color 0A 
echo. 
echo ================================================ 
echo   P2P FILE SHARING SYSTEM 
echo ================================================ 
echo. 
echo Starting P2P server... 
echo. 
echo IMPORTANT: 
echo 1. Keep this window open while using the system 
echo 2. The web interface will open automatically 
echo 3. If it doesn't open, go to: http://127.0.0.1:8080 
echo. 
echo Starting in 3 seconds... 
timeout /t 3 /nobreak >nul 
echo. 
REM Start the web interface 
start http://127.0.0.1:8080 
REM Start the P2P server 
p2p.exe 
