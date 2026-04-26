# Genesi OS

Sistema operacional baseado em Linux com interface desktop moderna usando Wayland.

---

## 🚨 ATUALIZAÇÃO IMPORTANTE - Solução WM Crash

O Genesi WM estava crashando com erro `EventLoopCreation`. 

**✅ SOLUÇÃO IMPLEMENTADA**: Removemos o WM e rodamos o Desktop diretamente no Sway.

### 🚀 Para Testar AGORA

```bash
cd ~/GenesiOS
sudo bash rebuild-iso.sh
```

### 📚 Documentação da Solução

- **[QUICK-START.md](QUICK-START.md)** ⚡ Início rápido
- **[RESUMAO.md](RESUMAO.md)** 📝 Resumo direto
- **[TESTE-RAPIDO.md](TESTE-RAPIDO.md)** 📖 Instruções detalhadas
- **[CHECKLIST-TESTE.md](CHECKLIST-TESTE.md)** ✅ Passo a passo
- **[INDICE-DOCUMENTACAO.md](INDICE-DOCUMENTACAO.md)** 📚 Índice completo

### 🔧 Ferramentas de Diagnóstico

- **[diagnostico-sway.sh](diagnostico-sway.sh)** - Script de diagnóstico automático
- **[COMANDOS-UTEIS.md](COMANDOS-UTEIS.md)** - Comandos para debugging

---

## 🚀 Início Rápido

### Desenvolvimento (WSL/VM Ubuntu)

```bash
# 1. Setup (primeira vez)
bash setup-wsl.sh

# 2. Rodar
bash run-genesi.sh

# 3. Parar
bash stop-genesi.sh
# ou
./cleanup
```

### Criar ISO (VM Ubuntu - NÃO funciona no WSL!)

```bash
# Primeira vez (build completo - 10-20 min)
sudo ./build-iso.sh

# Rebuilds rápidos depois (5-10 min)
sudo ./rebuild-iso.sh

# ISO estará em: GenesiOS-YYYYMMDD-HHMM.iso
```

## 📋 Documentação

- `CRIAR-ISO.md` - Como criar ISO bootável
- `GUIA-RAPIDO.md` - Guia rápido de uso
- `SOLUCAO-PROBLEMAS.md` - Troubleshooting
- `WSL-SETUP-GUIDE.md` - Setup detalhado WSL
- `genesi-desktop/FIREFOX-SSD-FIX.md` - Fix Firefox topbar dupla

## 🦊 Firefox na ISO

**Dúvida comum:** "Firefox abre no Ubuntu na VM, vai abrir no Genesi OS na ISO?"

**Resposta:** SIM! No desenvolvimento, Firefox abre no Ubuntu porque Ubuntu está "por baixo". Na ISO, Genesi OS é o único sistema, então Firefox SEMPRE abre dentro dele com 1 topbar apenas.

## 🛠️ Requisitos

### Desenvolvimento (WSL/Linux)
- Ubuntu 22.04+
- Rust + Cargo
- Node.js 20+
- Dependências Tauri (instaladas pelo setup-wsl.sh)

### Criar ISO (VM Ubuntu)
- Ubuntu 22.04 Desktop (VM)
- 30GB espaço livre
- 4GB RAM mínimo
- **NÃO funciona no WSL!**

## 🎯 Estrutura

```
GenesiOS/
├── genesi-desktop/          # Desktop (Tauri + React)
│   ├── genesi-wm/          # Window Manager (Wayland)
│   ├── src/                # Frontend React
│   └── src-tauri/          # Backend Rust
├── build-iso.sh            # Cria ISO bootável
├── run-genesi.sh           # Roda em desenvolvimento
└── setup-wsl.sh            # Instala dependências
```

## 🐛 Problemas Comuns

### Porta 1420 ocupada
```bash
./cleanup
bash run-genesi.sh
```

### Firefox com 2 topbars (desenvolvimento)
Normal! Na ISO terá apenas 1 topbar.

### Erro GTK no WSL
```bash
bash setup-wsl.sh
```

### ISO não boota
Veja `CRIAR-ISO.md` - Seção Troubleshooting

## 📊 Desenvolvimento vs ISO

| Aspecto | Desenvolvimento | ISO Bootada |
|---------|----------------|-------------|
| Sistema | Ubuntu + Genesi | Apenas Genesi |
| Firefox | Abre no Ubuntu | Abre no Genesi ✅ |
| Topbar | Dupla (visual) | Única ✅ |
| Performance | Mais lenta | Nativa ✅ |

## 🎉 Resultado Final

Quando bootado da ISO:
- ✅ Genesi OS único sistema
- ✅ Firefox integrado (1 topbar)
- ✅ Janelas gerenciadas pelo Genesi WM
- ✅ Performance nativa
- ✅ Pronto para distribuir!
