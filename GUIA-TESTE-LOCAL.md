# Guia de Teste Local (Sem Criar ISO)

## Visão Geral

Você pode testar o Genesi OS **antes** de criar a ISO, compilando e rodando localmente no Ubuntu.

## Pré-requisitos

- Ubuntu (VM ou nativo) - **NÃO funciona no WSL**
- Dependências instaladas (execute `setup-ubuntu-desktop.sh`)

## Passo 1: Compilar

No Ubuntu (VM):

```bash
cd ~/GenesiOS
bash test-genesi-local.sh
```

Isso vai:
- ✅ Verificar dependências
- ✅ Compilar Genesi WM
- ✅ Compilar Genesi Desktop
- ✅ Gerar binários em `target/release/`

**Tempo**: 5-10 minutos (primeira vez)

## Passo 2: Escolher Modo de Teste

### OPÇÃO A: Teste Rápido (Modo Janela) ⚡

**Mais fácil e rápido** - Roda dentro do ambiente gráfico atual

```bash
bash test-genesi-windowed.sh
```

**O que acontece:**
1. Weston abre em uma janela
2. Genesi WM roda dentro do Weston
3. Genesi Desktop aparece
4. Você pode testar a interface

**Vantagens:**
- ✅ Não precisa sair do ambiente gráfico
- ✅ Pode usar outros programas ao mesmo tempo
- ✅ Fácil de debugar

**Desvantagens:**
- ⚠️ Não é exatamente como a ISO (roda em janela)
- ⚠️ Performance pode ser diferente

### OPÇÃO B: Teste Completo (Modo TTY) 🎯

**Mais realista** - Simula exatamente como será na ISO

```bash
# 1. Saia do ambiente gráfico
sudo systemctl stop gdm3  # ou lightdm/sddm

# 2. Vá para TTY1
# Pressione: Ctrl+Alt+F1

# 3. Logue com seu usuário

# 4. Execute o teste
cd ~/GenesiOS
bash test-genesi-tty.sh
```

**O que acontece:**
1. Weston roda direto no DRM (hardware)
2. Genesi WM roda dentro do Weston
3. Genesi Desktop aparece em tela cheia
4. Exatamente como será na ISO

**Vantagens:**
- ✅ Teste realista (igual à ISO)
- ✅ Performance real
- ✅ Testa acesso ao hardware

**Desvantagens:**
- ⚠️ Precisa sair do ambiente gráfico
- ⚠️ Mais trabalhoso

## Estrutura dos Testes

### Teste Rápido (Janela)
```
Seu Desktop (GNOME/KDE/etc)
     ↓
Weston (janela) → wayland-test-0
     ↓
Genesi WM → wayland-test-1
     ↓
Genesi Desktop
```

### Teste Completo (TTY)
```
Hardware (DRM/KMS)
     ↓
Weston → wayland-0
     ↓
Genesi WM → wayland-1
     ↓
Genesi Desktop
```

## Troubleshooting

### Erro: "WM não compilado"

**Solução:**
```bash
bash test-genesi-local.sh
```

### Erro: "Weston falhou ao iniciar" (Modo Janela)

**Causa**: Weston não instalado

**Solução:**
```bash
sudo apt update
sudo apt install -y weston
```

### Erro: "Weston falhou ao iniciar" (Modo TTY)

**Causas possíveis:**
1. Não está em TTY (ainda em ambiente gráfico)
2. Outro compositor rodando

**Solução:**
```bash
# Pare o ambiente gráfico
sudo systemctl stop gdm3  # ou lightdm/sddm

# Vá para TTY1
# Ctrl+Alt+F1
```

### Erro: "Socket do WM não encontrado"

**Causa**: WM não conseguiu criar socket

**Solução**: Veja o log:
```bash
cat /tmp/genesi-test-*.log
# ou
cat /tmp/genesi-tty-test.log
```

### Desktop não aparece

**Solução**: Verifique os logs:
```bash
# Último teste
cat /tmp/genesi-test-*.log | tail -50

# Ou veja todos os logs
ls -lt /tmp/genesi-*.log
```

## Comparação: Teste vs ISO

| Aspecto | Teste Local | ISO |
|---------|-------------|-----|
| Compilação | Manual | Automática |
| Ambiente | Ubuntu existente | Sistema limpo |
| Weston | Instalado manualmente | Incluído na ISO |
| Logs | `/tmp/genesi-*.log` | `/tmp/genesi-startup.log` |
| Reiniciar | Só rodar script | Reboot da VM |

## Quando Usar Cada Método

### Use Teste Local quando:
- ✅ Desenvolvimento rápido
- ✅ Debugar problemas
- ✅ Testar mudanças pequenas
- ✅ Não quer esperar build da ISO

### Use ISO quando:
- ✅ Teste final antes de distribuir
- ✅ Verificar boot completo
- ✅ Testar em hardware diferente
- ✅ Validar sistema completo

## Próximos Passos

Depois de testar localmente e confirmar que funciona:

```bash
# Criar a ISO
bash build-iso.sh

# Ou rebuild rápido (se já criou antes)
bash rebuild-iso.sh
```

## Comandos Úteis

### Ver processos rodando:
```bash
ps aux | grep -E "weston|genesi"
```

### Matar tudo:
```bash
killall genesi-desktop genesi-wm weston
```

### Limpar sockets:
```bash
rm -f $XDG_RUNTIME_DIR/wayland-*
```

### Voltar ao ambiente gráfico (após teste TTY):
```bash
sudo systemctl start gdm3  # ou lightdm/sddm
```
