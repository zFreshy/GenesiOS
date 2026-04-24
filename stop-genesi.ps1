# Script para parar completamente o Genesi OS
# Uso: .\stop-genesi.ps1

Write-Host ""
Write-Host "🛑 Parando Genesi OS..." -ForegroundColor Yellow
Write-Host ""

# 1. Mata processos do Desktop Environment
Write-Host "[1/3] Parando Desktop Environment..." -ForegroundColor Cyan
Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force -ErrorAction SilentlyContinue
Get-Process | Where-Object {$_.ProcessName -like "*tauri*"} | Stop-Process -Force -ErrorAction SilentlyContinue
Write-Host "      ✓ Desktop parado" -ForegroundColor Green

# 2. Mata processos do Window Manager no WSL
Write-Host "[2/3] Parando Window Manager (WSL)..." -ForegroundColor Cyan
try {
    wsl pkill -9 genesi-wm 2>$null
    wsl pkill -9 cargo 2>$null
    Write-Host "      ✓ Window Manager parado" -ForegroundColor Green
} catch {
    Write-Host "      ⚠ WSL não está rodando" -ForegroundColor Yellow
}

# 3. Para o WSL completamente (isso vai matar o vmmemWSL)
Write-Host "[3/3] Parando WSL (vmmemWSL)..." -ForegroundColor Cyan
try {
    wsl --shutdown 2>$null
    Start-Sleep -Seconds 2
    
    # Verifica se o vmmemWSL ainda está rodando
    $vmmem = Get-Process -Name "vmmemWSL" -ErrorAction SilentlyContinue
    if ($vmmem) {
        Write-Host "      ⚠ vmmemWSL ainda está rodando (pode ser outro processo WSL)" -ForegroundColor Yellow
    } else {
        Write-Host "      ✓ WSL parado" -ForegroundColor Green
    }
} catch {
    Write-Host "      ⚠ Erro ao parar WSL: $_" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "✅ Genesi OS parado!" -ForegroundColor Green
Write-Host ""

# Mostra processos restantes relacionados
$remaining = Get-Process | Where-Object {$_.ProcessName -like "*genesi*" -or $_.ProcessName -like "*vmmem*"}
if ($remaining) {
    Write-Host "⚠ Processos ainda rodando:" -ForegroundColor Yellow
    $remaining | Format-Table ProcessName, Id, CPU -AutoSize
    Write-Host ""
    Write-Host "💡 Para forçar o fechamento do vmmemWSL:" -ForegroundColor Cyan
    Write-Host "   1. Feche todos os terminais WSL/Ubuntu" -ForegroundColor White
    Write-Host "   2. Execute: wsl --shutdown" -ForegroundColor White
    Write-Host "   3. Aguarde alguns segundos" -ForegroundColor White
} else {
    Write-Host "✓ Nenhum processo relacionado rodando" -ForegroundColor Green
}

Write-Host ""
