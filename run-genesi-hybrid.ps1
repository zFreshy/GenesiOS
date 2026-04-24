# Script híbrido: WM no WSL + Desktop no Windows
# Uso: .\run-genesi-hybrid.ps1

Write-Host "🚀 Iniciando Genesi OS (Modo Híbrido)" -ForegroundColor Cyan
Write-Host ""

# Compila nocsd no WSL
Write-Host "📦 Compilando nocsd.so..." -ForegroundColor Yellow
try {
    wsl cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so genesi-desktop/genesi-wm/nocsd.c 2>$null
    wsl echo "/tmp/genesi_nocsd.so" > /tmp/genesi-nocsd-path.txt 2>$null
    Write-Host "✓ nocsd.so compilado" -ForegroundColor Green
} catch {
    Write-Host "⚠ nocsd.c não compilado" -ForegroundColor Yellow
}

Write-Host ""

# Cleanup function
$cleanup = {
    Write-Host ""
    Write-Host "🛑 Parando Genesi OS..." -ForegroundColor Yellow
    
    # Para processos Windows
    Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    
    # Para processos WSL
    wsl pkill -9 genesi-wm 2>$null
    wsl pkill -9 cargo 2>$null
    
    Write-Host "✓ Processos finalizados" -ForegroundColor Green
}

Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action $cleanup | Out-Null

try {
    # 1. Inicia WM no WSL em background
    Write-Host "🪟 Iniciando Window Manager no WSL..." -ForegroundColor Cyan
    
    $wmJob = Start-Job -ScriptBlock {
        wsl bash -c "cd genesi-desktop/genesi-wm && cargo run --release 2>&1"
    }
    
    Start-Sleep -Seconds 3
    
    if ($wmJob.State -eq "Running") {
        Write-Host "✓ Window Manager rodando no WSL" -ForegroundColor Green
    } else {
        Write-Host "⚠ Window Manager não iniciou (continuando sem ele)" -ForegroundColor Yellow
    }
    
    Write-Host ""
    
    # 2. Inicia Desktop no Windows
    Write-Host "🖥️  Iniciando Desktop Environment no Windows..." -ForegroundColor Cyan
    Push-Location genesi-desktop
    
    if (-not (Test-Path "node_modules")) {
        Write-Host "📦 Instalando dependências npm..." -ForegroundColor Yellow
        npm install
    }
    
    Write-Host ""
    Write-Host "✓ Genesi OS iniciado!" -ForegroundColor Green
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host "  Para parar: Pressione Ctrl+C" -ForegroundColor White
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host ""
    
    npm run tauri dev
    
} catch {
    Write-Host "❌ Erro: $_" -ForegroundColor Red
} finally {
    Pop-Location
    
    # Cleanup
    Write-Host ""
    Write-Host "🛑 Parando Genesi OS..." -ForegroundColor Yellow
    
    if ($wmJob) {
        Stop-Job -Job $wmJob -ErrorAction SilentlyContinue
        Remove-Job -Job $wmJob -Force -ErrorAction SilentlyContinue
    }
    
    Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    wsl pkill -9 genesi-wm 2>$null
    wsl pkill -9 cargo 2>$null
    
    Write-Host "✓ Processos finalizados" -ForegroundColor Green
}
