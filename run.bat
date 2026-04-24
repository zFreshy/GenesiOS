@echo off
REM Script simples para rodar Genesi OS no Windows
REM Uso: run.bat

echo.
echo ========================================
echo    Genesi OS - Startup
echo ========================================
echo.

cd genesi-desktop

REM Instala dependências se necessário
if not exist "node_modules" (
    echo [*] Instalando dependencias npm...
    call npm install
    echo.
)

echo [OK] Genesi OS iniciado!
echo.
echo Para fechar: Feche a janela ou pressione Ctrl+C
echo.

REM Roda o Tauri
call npm run tauri dev

cd ..
