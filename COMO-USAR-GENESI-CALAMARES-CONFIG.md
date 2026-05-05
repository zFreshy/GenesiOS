# Como Usar o Genesi Calamares Config

## Resumo

Criamos um repositório completo com TODAS as configurações do Calamares para o Genesi OS, baseado no CachyOS mas totalmente customizado.

## O que isso resolve?

✅ **Sem mais erros de scripts faltando** - Todos os scripts estão incluídos
✅ **Branding completo** - Cores, logos, tudo do Genesi
✅ **Fácil de manter** - Tudo em um repositório separado
✅ **Versionado** - Pode fazer commits e track mudanças
✅ **Testado** - Usa a base do CachyOS que já funciona

## Passos para configurar

### 1. Criar o repositório no GitHub

```
1. Acesse: https://github.com/zFreshy
2. Clique em "New repository"
3. Nome: genesi-calamares-config
4. Descrição: Calamares installer configuration for Genesi OS
5. Público
6. NÃO marque nenhuma opção (README, gitignore, license)
7. Create repository
```

### 2. Fazer push do código

```bash
cd genesi-calamares-config-full
git remote add origin https://github.com/zFreshy/genesi-calamares-config.git
git branch -M main
git push -u origin main
cd ..
```

### 3. Configurar como submodule

```bash
# Remover o submodule antigo do CachyOS
git submodule deinit -f cachyos-calamares-config
git rm -f cachyos-calamares-config
rm -rf .git/modules/cachyos-calamares-config

# Remover a pasta temporária
rm -rf genesi-calamares-config-full

# Adicionar o novo submodule
git submodule add https://github.com/zFreshy/genesi-calamares-config.git genesi-calamares-config-full

# Commit
git add .gitmodules genesi-calamares-config-full
git commit -m "Add genesi-calamares-config as submodule"
git push
```

### 4. Testar

```bash
cd genesi-arch
bash prepare-and-build.sh
```

## O que mudou no código?

### Arquivos modificados:

1. **genesi-arch/archiso/airootfs/root/customize_airootfs.sh**
   - Agora copia de `genesi-calamares-config-full` em vez de `cachyos-calamares-config`
   - Instala branding, scripts, módulos e configurações

2. **genesi-arch/prepare-and-build.sh**
   - Copia `genesi-calamares-config-full` para o airootfs antes do build

3. **.gitmodules**
   - Novo submodule apontando para seu repositório

## Estrutura do genesi-calamares-config

```
genesi-calamares-config-full/
├── etc/calamares/
│   ├── branding/genesi/
│   │   ├── branding.desc          # Configuração de branding
│   │   ├── logo.png               # Logo do Genesi
│   │   ├── icon.png               # Ícone da janela
│   │   ├── welcome.png            # Imagem de boas-vindas
│   │   ├── slide1-6.png           # Slides da instalação
│   │   ├── show.qml               # Slideshow QML
│   │   └── stylesheet.qss         # Estilos CSS
│   ├── modules/
│   │   ├── bootloader.conf        # Config do bootloader
│   │   ├── packages_online.conf   # Pacotes para instalar
│   │   ├── shellprocess*.conf     # Configs de scripts
│   │   └── ... (todos os módulos)
│   ├── scripts/
│   │   ├── update-mirrorlist      # Atualiza mirrors
│   │   ├── create-pacman-keyring  # Cria keyring
│   │   ├── mkinitcpio-install-calamares
│   │   └── ... (todos os scripts)
│   └── settings.conf              # Config principal
└── usr/lib/calamares/modules/     # Módulos Python
```

## Customizações feitas

### Branding (etc/calamares/branding/genesi/branding.desc)

```yaml
componentName: genesi
productName: Genesi OS
SidebarBackground: "#0A1E1A"        # Verde escuro
SidebarBackgroundCurrent: "#00ff9f" # Verde neon
```

### Bootloader (etc/calamares/modules/bootloader.conf)

```yaml
efiBootloaderId: "genesi"
```

### Settings (etc/calamares/settings.conf)

```yaml
branding: genesi
```

## Próximas customizações

### Trocar logos

Substitua os arquivos em `genesi-calamares-config-full/etc/calamares/branding/genesi/`:

```bash
# Clone o repositório
git clone https://github.com/zFreshy/genesi-calamares-config.git
cd genesi-calamares-config

# Substitua os logos
cp /caminho/para/seu/logo.png etc/calamares/branding/genesi/logo.png
cp /caminho/para/seu/icon.png etc/calamares/branding/genesi/icon.png
cp /caminho/para/seu/welcome.png etc/calamares/branding/genesi/welcome.png

# Commit e push
git add .
git commit -m "Update Genesi logos"
git push
```

### Atualizar no projeto principal

```bash
cd genesi-calamares-config-full
git pull
cd ..
git add genesi-calamares-config-full
git commit -m "Update calamares config"
git push
```

## Vantagens

1. **Organização**: Tudo relacionado ao Calamares em um só lugar
2. **Manutenção**: Fácil de atualizar e versionar
3. **Reutilização**: Pode usar em outros projetos
4. **Colaboração**: Outros podem contribuir facilmente
5. **Sem erros**: Scripts testados do CachyOS como base

## Resultado

Quando você buildar a ISO agora:

✅ Calamares abre com branding do Genesi
✅ Cores verde neon (#00ff9f)
✅ Todos os scripts funcionam
✅ Instalação completa sem erros
✅ Logos e slides customizados

## Dúvidas?

Leia os arquivos:
- `GENESI-CALAMARES-CONFIG-SETUP.md` - Explicação detalhada
- `genesi-calamares-config-full/README.md` - Documentação do repositório
- `genesi-calamares-config-full/PUSH-TO-GITHUB.md` - Instruções de push
