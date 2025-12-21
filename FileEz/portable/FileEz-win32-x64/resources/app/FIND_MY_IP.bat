@echo off 
title Find My IP Address 
echo ================================================ 
echo   Finding Your IP Address 
echo ================================================ 
echo. 
echo Your IP addresses: 
ipconfig | findstr /i "IPv4" 
echo. 
echo Give one of these IP addresses to your friend! 
echo They should add: YOUR_IP:8081 to their system 
echo. 
pause 
