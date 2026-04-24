@echo off
REM Script para parar completamente o Genesi OS
REM Uso: stop-genesi.bat

echo.
echo ========================================
echo    Parando Genesi OS
echo ========================================
echo.

REM 1. Mata processos do Desktop
echo [1/3] Parando Desktop Environment...
taskkill /F /IM "genesi-desktop.exe" 2>nul
taskkill /F /IM "tauri.exe" 2>nul
echo       [OK] Desktop parado
echo.

REM 2. Mata processos do WM no WSL
echo [2/3] Parando Window Manager ^(WSL^)...
wsl pkill -9 genesi-wm 2>nul
wsl pkill -9 cargo 2>nul
echo       [OK] Window Manager parado
echo.

REM 3. Para o WSL
echo [3/3] Parando WSL ^(vmmemWSL^)...
wsl --shutdown 2>nul
timeout /t 2 /nobreak >nul
echo       [OK] WSL parado
echo.

echo ========================================
echo    Genesi OS parado!
echo ========================================
echo.
echo [!] Se o vmmemWSL ainda estiver rodando:
echo     1. Feche todos os terminais WSL/Ubuntu
echo     2. Execute: wsl --shutdown
echo     3. Aguarde alguns segundos
echo.

pause
