@echo off
REM Script Batch para rodar o Genesi OS no Windows
REM Uso: run-genesi.bat

echo.
echo ========================================
echo    Genesi OS - Startup Script
echo ========================================
echo.

REM Compila nocsd.so no WSL (se disponível)
if exist "genesi-desktop\genesi-wm\nocsd.c" (
    echo [*] Compilando nocsd.so...
    wsl cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so genesi-desktop/genesi-wm/nocsd.c 2>nul
    if %ERRORLEVEL% EQU 0 (
        echo [OK] nocsd.so compilado
    ) else (
        echo [!] nocsd.c nao compilado ^(WSL nao disponivel^)
    )
    echo.
)

REM Inicia o Desktop Environment
echo [*] Iniciando Desktop Environment...
cd genesi-desktop

REM Instala dependências se necessário
if not exist "node_modules" (
    echo [*] Instalando dependencias npm...
    call npm install
    echo.
)

echo.
echo [OK] Genesi OS iniciado!
echo.
echo Para fechar: Pressione Ctrl+C ou feche a janela
echo.

REM Roda o Tauri
call npm run tauri dev

REM Cleanup ao sair
echo.
echo [*] Parando Genesi OS...
taskkill /F /IM "genesi-desktop.exe" 2>nul
taskkill /F /IM "cargo.exe" 2>nul

REM Para o WSL se estiver rodando
echo [*] Parando WSL (Window Manager)...
wsl pkill -9 genesi-wm 2>nul
wsl pkill -9 cargo 2>nul

echo [OK] Processos finalizados
echo.
echo [!] Se o vmmemWSL ainda estiver rodando, execute:
echo     wsl --shutdown
echo.
cd ..
