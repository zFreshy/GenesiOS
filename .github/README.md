# Genesi OS - CI/CD Workflows

Este diretório contém os workflows do GitHub Actions para build e testes automatizados do Genesi OS.

## 📋 Workflows Disponíveis

### 1. Build ISO (`build-iso.yml`)

**Quando executa:**
- Push para branches `main`, `master` ou `develop`
- Pull requests para `main` ou `master`
- Manualmente via GitHub Actions UI

**O que faz:**
1. ✅ Configura ambiente Ubuntu 22.04
2. 📦 Instala todas as dependências (debootstrap, squashfs-tools, etc.)
3. 🦀 Instala Rust e compila componentes
4. 🏗️ Executa `build-iso.sh` para criar a ISO completa
5. 📤 Faz upload da ISO como artifact (disponível por 30 dias)
6. 🏷️ Cria release automático se for uma tag

**Tempo estimado:** ~45-60 minutos

**Artifact gerado:** `genesi-os-iso` (arquivo `.iso`)

### 2. Test Build (`test-build.yml`)

**Quando executa:**
- Push para branches principais
- Pull requests
- Mudanças em `genesi-desktop/**`

**O que faz:**
1. ✅ Verifica sintaxe Rust (`cargo check`)
2. 🧪 Executa testes (`cargo test`)
3. 🔨 Compila componentes individuais
4. ✅ Valida que binários foram gerados corretamente

**Tempo estimado:** ~10-15 minutos

## 🚀 Como Usar

### Executar Build Manualmente

1. Vá para **Actions** no GitHub
2. Selecione **Build Genesi OS ISO**
3. Clique em **Run workflow**
4. Escolha a branch
5. Clique em **Run workflow**

### Baixar ISO Gerada

1. Vá para **Actions** no GitHub
2. Clique no workflow executado
3. Role até **Artifacts**
4. Baixe `genesi-os-iso`

### Criar Release com ISO

Para criar um release automático com a ISO:

```bash
# Crie uma tag
git tag v1.0.0
git push origin v1.0.0
```

O workflow irá:
- Buildar a ISO
- Criar um release no GitHub
- Anexar a ISO ao release
- Gerar SHA256 checksum

## 🔧 Configuração

### Secrets Necessários

Nenhum secret adicional é necessário. O workflow usa `GITHUB_TOKEN` automático.

### Modificar Workflows

Para editar os workflows:

```bash
# Editar workflow de build
vim .github/workflows/build-iso.yml

# Editar workflow de testes
vim .github/workflows/test-build.yml
```

### Cache

Os workflows usam cache do Cargo para acelerar builds:
- Cache de dependências Rust
- Cache de compilações anteriores
- Chave baseada em `Cargo.lock`

## 📊 Status dos Workflows

Adicione badges no README principal:

```markdown
![Build ISO](https://github.com/SEU_USUARIO/GenesiOS/actions/workflows/build-iso.yml/badge.svg)
![Test Build](https://github.com/SEU_USUARIO/GenesiOS/actions/workflows/test-build.yml/badge.svg)
```

## 🐛 Troubleshooting

### Build falha por falta de espaço

O workflow já limpa espaço automaticamente, mas se ainda falhar:
- Reduza o tamanho do sistema base
- Use compressão mais agressiva no squashfs
- Remova pacotes desnecessários

### Timeout no build

Se o build ultrapassar 6 horas (limite do GitHub Actions):
- Use cache mais agressivo
- Compile componentes em paralelo
- Considere usar self-hosted runners

### Dependências faltando

Se faltar alguma dependência:
1. Edite `.github/workflows/build-iso.yml`
2. Adicione o pacote na seção `apt-get install`
3. Commit e push

## 📝 Logs

Para ver logs detalhados:
1. Vá para **Actions**
2. Clique no workflow
3. Clique em cada step para expandir logs

Logs importantes:
- `🔨 Compilar componentes Rust` - erros de compilação
- `🏗️ Build ISO` - erros no debootstrap/squashfs
- `📊 Informações da ISO` - tamanho e checksum

## 🔐 Segurança

- Workflows rodam em ambiente isolado
- Não expõem secrets
- ISO gerada é verificável via SHA256
- Artifacts expiram após 30 dias

## 📚 Recursos

- [GitHub Actions Docs](https://docs.github.com/en/actions)
- [Rust CI/CD Best Practices](https://doc.rust-lang.org/cargo/guide/continuous-integration.html)
- [Ubuntu Debootstrap](https://wiki.debian.org/Debootstrap)
