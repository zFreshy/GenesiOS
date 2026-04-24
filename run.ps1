# Script simples para rodar Genesi OS no Windows
# Uso: .\run.ps1

Write-Host ""
Write-Host "🚀 Iniciando Genesi OS..." -ForegroundColor Cyan
Write-Host ""

# Vai para a pasta do desktop
Push-Location genesi-desktop

# Instala dependências se necessário
if (-not (Test-Path "node_modules")) {
    Write-Host "📦 Instalando dependências npm..." -ForegroundColor Yellow
    npm install
    Write-Host ""
}

Write-Host "✓ Genesi OS iniciado!" -ForegroundColor Green
Write-Host ""
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  Para parar: Feche a janela ou Ctrl+C" -ForegroundColor White
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

# Roda o Tauri
npm run tauri dev

Pop-Location
