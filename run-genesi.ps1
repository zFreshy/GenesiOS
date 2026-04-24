# Script PowerShell para rodar o Genesi OS no Windows
# Uso: .\run-genesi.ps1

Write-Host "🚀 Iniciando Genesi OS..." -ForegroundColor Cyan
Write-Host ""

# Compila o nocsd.so (se estiver no WSL/Linux)
if (Test-Path "genesi-desktop/genesi-wm/nocsd.c") {
    Write-Host "📦 Compilando nocsd.so..." -ForegroundColor Yellow
    try {
        wsl cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so genesi-desktop/genesi-wm/nocsd.c 2>$null
        wsl echo "/tmp/genesi_nocsd.so" > /tmp/genesi-nocsd-path.txt 2>$null
        Write-Host "✓ nocsd.so compilado" -ForegroundColor Green
    } catch {
        Write-Host "⚠ nocsd.c não compilado (WSL não disponível)" -ForegroundColor Yellow
    }
}

Write-Host ""

# Função de cleanup
$cleanup = {
    Write-Host ""
    Write-Host "🛑 Parando Genesi OS..." -ForegroundColor Yellow
    Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    Get-Process | Where-Object {$_.ProcessName -like "*tauri*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    Write-Host "✓ Processos finalizados" -ForegroundColor Green
    exit 0
}

# Registra handler para Ctrl+C
Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action $cleanup | Out-Null
[Console]::TreatControlCAsInput = $false

try {
    # 1. Inicia o Window Manager em background (se estiver no Linux/WSL)
    Write-Host "🪟 Iniciando Window Manager (genesi-wm)..." -ForegroundColor Cyan
    $wmJob = $null
    
    if (Test-Path "genesi-desktop/genesi-wm/Cargo.toml") {
        try {
            Push-Location "genesi-desktop/genesi-wm"
            $wmJob = Start-Job -ScriptBlock {
                Set-Location $using:PWD
                cargo run --release 2>&1
            }
            Pop-Location
            
            Start-Sleep -Seconds 2
            
            if ($wmJob.State -eq "Running") {
                Write-Host "✓ Window Manager rodando (Job ID: $($wmJob.Id))" -ForegroundColor Green
            } else {
                Write-Host "⚠ Window Manager não iniciou (rodando apenas Desktop)" -ForegroundColor Yellow
            }
        } catch {
            Write-Host "⚠ Erro ao iniciar WM: $_" -ForegroundColor Yellow
        }
    }
    
    Write-Host ""
    
    # 2. Inicia o Desktop Environment
    Write-Host "🖥️  Iniciando Desktop Environment (genesi-desktop)..." -ForegroundColor Cyan
    Push-Location "genesi-desktop"
    
    # Instala dependências se necessário
    if (-not (Test-Path "node_modules")) {
        Write-Host "📦 Instalando dependências npm..." -ForegroundColor Yellow
        npm install
    }
    
    # Roda o Tauri
    Write-Host ""
    Write-Host "✓ Genesi OS iniciado! Pressione Ctrl+C para parar." -ForegroundColor Green
    Write-Host ""
    
    npm run tauri dev
    
} catch {
    Write-Host "❌ Erro: $_" -ForegroundColor Red
} finally {
    Pop-Location
    
    # Cleanup ao sair
    Write-Host ""
    Write-Host "🛑 Parando Genesi OS..." -ForegroundColor Yellow
    
    # Para o WM job se existir
    if ($wmJob) {
        Stop-Job -Job $wmJob -ErrorAction SilentlyContinue
        Remove-Job -Job $wmJob -Force -ErrorAction SilentlyContinue
    }
    
    # Mata qualquer processo restante
    Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    Get-Process | Where-Object {$_.ProcessName -like "*tauri*"} | Stop-Process -Force -ErrorAction SilentlyContinue
    
    Write-Host "✓ Processos finalizados" -ForegroundColor Green
}
