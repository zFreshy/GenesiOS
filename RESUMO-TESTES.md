# Resumo: Como Testar o Genesi OS

## 🎯 Fluxo de Trabalho

```
┌─────────────────────────────────────────────────────────────┐
│                    DESENVOLVIMENTO                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1️⃣  COMPILAR                                               │
│     bash test-genesi-local.sh                              │
│     ↓                                                       │
│     Compila WM + Desktop                                   │
│     Tempo: 5-10 min                                        │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  2️⃣  TESTAR RÁPIDO (Modo Janela) ⚡                         │
│     bash test-genesi-windowed.sh                           │
│     ↓                                                       │
│     Roda em janela, fácil de debugar                       │
│     Tempo: Instantâneo                                     │
│                                                             │
│     OU                                                      │
│                                                             │
│  2️⃣  TESTAR COMPLETO (Modo TTY) 🎯                          │
│     sudo systemctl stop gdm3                               │
│     Ctrl+Alt+F1                                            │
│     bash test-genesi-tty.sh                                │
│     ↓                                                       │
│     Simula ISO, teste realista                             │
│     Tempo: Instantâneo                                     │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  3️⃣  CRIAR ISO (Quando tudo funcionar)                      │
│     bash build-iso.sh                                      │
│     ↓                                                       │
│     ISO completa para distribuição                         │
│     Tempo: 15-30 min                                       │
│                                                             │
│     OU (se já criou antes)                                 │
│                                                             │
│     bash rebuild-iso.sh                                    │
│     ↓                                                       │
│     Rebuild rápido                                         │
│     Tempo: 3-5 min                                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

## 📋 Comandos Rápidos

### No Ubuntu (VM):

```bash
# 1. Compilar tudo
cd ~/GenesiOS
bash test-genesi-local.sh

# 2a. Teste rápido (em janela)
bash test-genesi-windowed.sh

# 2b. Teste completo (TTY)
sudo systemctl stop gdm3
# Ctrl+Alt+F1
bash test-genesi-tty.sh

# 3. Criar ISO
bash build-iso.sh
```

## 🔄 Ciclo de Desenvolvimento

```
Editar código
    ↓
Compilar (test-genesi-local.sh)
    ↓
Testar rápido (test-genesi-windowed.sh)
    ↓
Funciona? ──→ NÃO ──→ Voltar para "Editar código"
    ↓
   SIM
    ↓
Testar completo (test-genesi-tty.sh)
    ↓
Funciona? ──→ NÃO ──→ Debugar e voltar
    ↓
   SIM
    ↓
Criar ISO (build-iso.sh)
    ↓
Testar ISO na VM
    ↓
Funciona? ──→ NÃO ──→ Voltar para "Editar código"
    ↓
   SIM
    ↓
✅ PRONTO PARA DISTRIBUIR!
```

## 🎨 Comparação Visual

### Teste Rápido (Janela)
```
┌─────────────────────────────────────────┐
│  Seu Desktop (GNOME/KDE)                │
│  ┌───────────────────────────────────┐  │
│  │  Weston (janela)                  │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │  Genesi WM                  │  │  │
│  │  │  ┌───────────────────────┐  │  │  │
│  │  │  │  Genesi Desktop       │  │  │  │
│  │  │  │  [Interface aqui]     │  │  │  │
│  │  │  └───────────────────────┘  │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### Teste Completo (TTY)
```
┌─────────────────────────────────────────┐
│  Hardware (Tela cheia)                  │
│  ┌───────────────────────────────────┐  │
│  │  Weston (DRM)                     │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │  Genesi WM                  │  │  │
│  │  │  ┌───────────────────────┐  │  │  │
│  │  │  │  Genesi Desktop       │  │  │  │
│  │  │  │  [Tela cheia]         │  │  │  │
│  │  │  └───────────────────────┘  │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### ISO (Produção)
```
┌─────────────────────────────────────────┐
│  Boot → GRUB → Kernel → Init            │
│  ↓                                      │
│  Weston (DRM) → wayland-0               │
│  ↓                                      │
│  Genesi WM → wayland-1                  │
│  ↓                                      │
│  Genesi Desktop (autostart)             │
│  ↓                                      │
│  [Sistema completo rodando]             │
└─────────────────────────────────────────┘
```

## ⏱️ Tempo de Cada Etapa

| Etapa | Primeira Vez | Depois |
|-------|--------------|--------|
| Compilar | 5-10 min | 1-2 min |
| Teste Janela | Instantâneo | Instantâneo |
| Teste TTY | Instantâneo | Instantâneo |
| Build ISO | 15-30 min | - |
| Rebuild ISO | - | 3-5 min |

## 🐛 Troubleshooting Rápido

### Erro ao compilar
```bash
# Instale dependências
bash setup-ubuntu-desktop.sh
```

### Weston não inicia
```bash
# Instale Weston
sudo apt install -y weston
```

### Desktop não conecta ao WM
```bash
# Veja os logs
cat /tmp/genesi-test-*.log
cat /tmp/genesi-tty-test.log
```

### Tela preta na ISO
```bash
# Dentro da ISO (Ctrl+Alt+F2)
cat /tmp/genesi-startup.log
```

## 📚 Documentação Completa

- `GUIA-TESTE-LOCAL.md` - Guia detalhado de testes locais
- `GUIA-REBUILD-RAPIDO.md` - Como fazer rebuild da ISO
- `CRIAR-ISO.md` - Como criar a ISO completa
- `CORRECAO-WM-CONNECTION.md` - Explicação técnica da correção

## 🎯 Recomendação

**Para desenvolvimento diário:**
1. Use `test-genesi-windowed.sh` (rápido e fácil)
2. Quando funcionar bem, teste com `test-genesi-tty.sh`
3. Só crie ISO quando tudo estiver funcionando

**Para distribuição:**
1. Teste completo com `test-genesi-tty.sh`
2. Crie ISO com `build-iso.sh`
3. Teste a ISO em VM limpa
4. Distribua! 🚀
