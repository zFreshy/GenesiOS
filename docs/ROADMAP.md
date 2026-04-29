# Genesi OS - Roadmap de Features

## Status Atual
- ✅ ISO bootável baseada no CachyOS
- ✅ KDE Plasma funcionando
- ✅ Build system completo
- ⬜ Tudo abaixo

---

## FASE 1: Identidade Visual (Prioridade Alta)
> Deixar o Genesi OS com cara própria

- [ ] Tema KDE Plasma customizado (cores, ícones, fontes)
- [ ] Wallpapers próprios do Genesi OS
- [ ] Tela de login (SDDM) com branding Genesi
- [ ] Splash screen de boot com logo Genesi
- [ ] "Genesi Hello" no lugar do "CachyOS Hello"
- [ ] Ícones e logo do Genesi OS
- [ ] Textos e links apontando pro Genesi (não CachyOS)

---

## FASE 2: AI Mode - Otimizações para IA Local (Diferencial Principal)
> Fazer o Genesi OS rodar IA local melhor que qualquer outro OS

### 2.1 Daemon "Genesi AI Optimizer" (genesi-aid)
Serviço systemd que monitora processos de IA e otimiza o sistema automaticamente.

- [ ] Detectar quando Ollama/llama.cpp/vLLM/LocalAI está rodando
- [ ] Ativar otimizações automaticamente quando IA está em uso
- [ ] Desativar otimizações quando IA para (voltar ao normal)
- [ ] Widget no Plasma mostrando status (AI Mode ON/OFF, VRAM uso, modelo carregado)

### 2.2 Gerenciamento de VRAM/RAM
- [ ] Detectar VRAM livre automaticamente
- [ ] Configurar split GPU/CPU ideal pro modelo (offloading parcial)
- [ ] Liberar VRAM de processos não essenciais quando IA roda (reduzir efeitos visuais do compositor)
- [ ] Usar `mlock` pra manter pesos do modelo na RAM sem swap
- [ ] Configurar `vm.swappiness=10` quando AI Mode ativo

### 2.3 Huge Pages para Modelos
- [ ] Configurar Transparent Huge Pages (THP) de 2MB para inferência
- [ ] Pré-alocar huge pages quando AI Mode é ativado
- [ ] Sysctl configs otimizados: `vm.nr_hugepages`, `vm.hugetlb_shm_group`

### 2.4 CPU Governor e Scheduler
- [ ] Mudar CPU governor pra `performance` quando inferência está rodando
- [ ] Desabilitar power saving nos cores usados pela IA
- [ ] CPU pinning: fixar threads de inferência em cores específicos (evitar cache thrashing)
- [ ] Usar o scheduler BORE do CachyOS com prioridade alta pra processos de IA

### 2.5 I/O Otimizado para Modelos
- [ ] Pré-cachear modelos frequentes na RAM com `vmtouch`
- [ ] Configurar scheduler de I/O pra priorizar leitura sequencial grande
- [ ] Otimizar readahead do kernel pra arquivos GGUF grandes

### 2.6 Toggle "AI Mode" no Plasma
- [ ] Widget na barra de tarefas: botão ON/OFF
- [ ] Quando ON: reduz efeitos visuais, ativa huge pages, CPU performance, prioriza IA
- [ ] Quando OFF: volta tudo ao normal
- [ ] Mostrar: VRAM usada, modelo carregado, tokens/segundo

### 2.7 MemPalace Integrado
O MemPalace (https://github.com/MemPalace/mempalace) é um sistema de memória local para IA.
Ele armazena conversas e contexto localmente com busca semântica, sem enviar nada pra nuvem.

Como integrar no Genesi OS:
- [ ] Pré-instalar MemPalace no sistema
- [ ] Configurar como serviço que roda em background
- [ ] IAs locais (Ollama, etc.) podem usar o MemPalace pra ter memória persistente
- [ ] Contexto de projetos do dev é indexado automaticamente
- [ ] Busca semântica: "por que mudamos pra GraphQL?" retorna a conversa exata
- [ ] Integrar com a IDE (VS Code/Zed) via MCP tools do MemPalace
- [ ] Widget no Plasma mostrando status do MemPalace (memórias indexadas, último sync)

Benefício: A IA local no Genesi OS tem memória de longo prazo. O dev conversa com a IA,
fecha tudo, e na próxima vez a IA lembra do contexto. Nenhum outro OS oferece isso nativamente.

---

## FASE 3: IDE e Ferramentas de Dev (Diferencial Secundário)

### 3.1 IDE Genesi (baseada em VS Code ou Zed)
- [ ] Fork do VS Code ou Zed com branding Genesi
- [ ] Tema Genesi pré-instalado
- [ ] Extensões pré-configuradas (Git, Docker, AI, linguagens populares)
- [ ] Integração nativa com o daemon de IA local
- [ ] Integração com MemPalace (contexto do projeto)
- [ ] Atalho no desktop e menu

### 3.2 Widget de Containers no Plasma
- [ ] Widget na barra de tarefas mostrando containers Docker rodando
- [ ] Start/Stop/Restart com um clique
- [ ] Ver logs do container
- [ ] Ver portas mapeadas
- [ ] Status de uso de CPU/RAM por container

### 3.3 Sandboxes de Projetos (Workspaces Isolados)
- [ ] Baseado em Distrobox/Toolbox
- [ ] Interface gráfica pra criar/gerenciar workspaces
- [ ] Templates: "Java + Spring Boot", "React + Vite", "Python + FastAPI", etc.
- [ ] Cada workspace tem suas dependências isoladas
- [ ] Integração com a IDE Genesi

### 3.4 Inspeção de Rede
- [ ] mitmproxy pré-instalado e configurado
- [ ] Interface gráfica simples pra interceptar requisições HTTP/HTTPS
- [ ] Atalho rápido pra ativar/desativar proxy de debug
- [ ] Integração com o widget de containers (ver tráfego por container)

### 3.5 Explorador de Banco de Dados
- [ ] Beekeeper Studio ou DBeaver pré-instalado
- [ ] Plugin do Dolphin (explorador de arquivos) pra conectar em bancos
- [ ] Suporte a PostgreSQL, MySQL, SQLite, MongoDB
- [ ] Visualização rápida de tabelas e dados

---

## FASE 4: Polimento e Distribuição

- [ ] Instalador Calamares customizado com branding Genesi
- [ ] Site oficial do Genesi OS
- [ ] Documentação completa
- [ ] Página de download com ISOs
- [ ] Comunidade (Discord/Forum)
- [ ] Atualizações automáticas (repositório próprio)

---

## Como Baixar e Testar IA Local no Genesi OS

### Método 1: Ollama (mais fácil)
```bash
# Instalar Ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Baixar um modelo
ollama pull llama3.2        # 2GB, leve
ollama pull codellama       # 4GB, bom pra código
ollama pull deepseek-coder  # 1.3GB, leve e bom pra código

# Rodar
ollama run llama3.2
# Digite sua pergunta e a IA responde localmente

# Usar via API (pra integrar com apps)
curl http://localhost:11434/api/generate -d '{"model":"llama3.2","prompt":"Hello"}'
```

### Método 2: llama.cpp (mais controle)
```bash
# Instalar
sudo pacman -S llama.cpp

# Baixar modelo GGUF do HuggingFace
wget https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q4_K_M.gguf

# Rodar
llama-cli -m llama-2-7b-chat.Q4_K_M.gguf -p "Hello, how are you?"

# Rodar como servidor (API compatível com OpenAI)
llama-server -m llama-2-7b-chat.Q4_K_M.gguf --port 8080
```

### Método 3: LocalAI (compatível com API OpenAI)
```bash
# Via Docker
docker run -p 8080:8080 localai/localai
```

### Testando Performance (pra documentação)
```bash
# Com Ollama - medir tokens por segundo
ollama run llama3.2 "Explain quantum computing" --verbose

# Com llama.cpp - benchmark
llama-bench -m modelo.gguf

# Monitorar uso de recursos durante inferência
# Terminal 1: rodar IA
# Terminal 2:
watch -n 1 nvidia-smi          # GPU (se tiver NVIDIA)
htop                            # CPU e RAM
```

### Comparação pra documentação
Rodar o mesmo prompt no Genesi OS (com AI Mode ON) e num Ubuntu/Fedora padrão.
Medir:
- Tokens por segundo
- Uso de RAM
- Tempo de carregamento do modelo
- Uso de VRAM

---

## Sobre o MemPalace

O MemPalace é um sistema de memória local para IA que:
- Armazena conversas como texto verbatim (sem resumir/alterar)
- Organiza em "wings" (projetos), "rooms" (tópicos), "drawers" (conteúdo)
- Busca semântica local (96.6% recall sem LLM, 98.4% com heurísticas)
- Tudo local, nada sai da máquina
- 29 ferramentas MCP pra integrar com qualquer IA

Como ajuda o Genesi OS:
1. **Memória persistente pra IA local**: O dev conversa com Ollama, fecha, e na próxima vez a IA lembra
2. **Contexto de projeto**: MemPalace indexa o código do projeto e a IA entende o contexto
3. **Zero cloud**: Tudo roda local, alinhado com a filosofia do Genesi OS
4. **Integração com IDE**: Via MCP tools, a IDE pode buscar contexto no MemPalace

Licença: MIT (compatível com GPL-3.0 do Genesi OS)
