@echo off 
echo ============================================ 
echo   P2P File Share - DOWNLOAD MODE 
echo ============================================ 
echo. 
echo First, edit peers.txt to add peer addresses! 
echo. 
set /p filename="Enter filename to download: " 
echo. 
echo Downloading... 
p2p_share.exe --mode=download --file=%filename% 
echo. 
echo File saved to: data\downloads\%filename% 
pause 
