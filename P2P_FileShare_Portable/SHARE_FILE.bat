@echo off 
echo ============================================ 
echo   P2P File Share - SHARE MODE 
echo ============================================ 
echo. 
echo Put your files in: data\shared\ 
echo. 
set /p filename="Enter filename to share: " 
set /p port="Enter port (default 8080): " 
if "%port%"=="" set port=8080 
echo. 
echo Starting server on port %port%... 
p2p_share.exe --mode=share --file=%filename% --port=%port% 
pause 
