@echo off 
echo ============================================ 
echo   P2P File Share - INTERACTIVE MODE 
echo ============================================ 
echo. 
echo Your IP Address: 
for /f "tokens=2 delims=:" %%a in ('ipconfig | findstr /c:"IPv4 Address"') do echo %%a 
echo. 
set /p port="Enter port (default 8080): " 
if "%port%"=="" set port=8080 
echo. 
p2p_share.exe --mode=hybrid --port=%port% 
