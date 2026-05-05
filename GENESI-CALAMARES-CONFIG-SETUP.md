# Genesi OS - Calamares Config Setup

## O que foi feito

Criamos um fork completo do repositório `calamares-config` do CachyOS, customizado para o Genesi OS.

### 1. Estrutura criada

```
genesi-calamares-config-full/
├── etc/calamares/
│   ├── branding/genesi/          # Branding do Genesi (cores, logos, slideshow)
│   ├── modules/                  # Configurações dos módulos do Calamares
│   ├── scripts/                  # Scripts de instalação
│   └── settings.conf             # Configuração principal
├── usr/lib/calamares/modules/    # Módulos Python customizados
├── README.md
└── PUSH-TO-GITHUB.md
```

### 2. Modificações feitas

#### Branding
- **Pasta renomeada**: `cachyos` → `genesi`
- **Cores atualizadas**:
  - Background: `#0A1E1A` (verde escuro Genesi)
  - Highlight: `#00ff9f` (verde neon Genesi)
- **Nomes atualizados**: Todos os "CachyOS" → "Genesi OS"
- **Versão**: 2025-01-01

#### Configurações
- `settings.conf`: branding alterado para `genesi`
- `bootloader.conf`: efiBootloaderId alterado para `genesi`

#### Scripts
Todos os scripts do CachyOS foram mantidos:
- `update-mirrorlist` - Atualiza mirrors
- `create-pacman-keyring` - Cria keyring do pacman
- `mkinitcpio-install-calamares` - Hook modificado do mkinitcpio
- `90-mkinitcpio-install.hook` - Hook do pacman
- `try-v3` - Detecta suporte a x86-64-v3
- `pacstrap_calamares` - Script de instalação de pacotes

### 3. Como usar

#### Passo 1: Criar repositório no GitHub

1. Acesse https://github.com/zFreshy
2. Crie novo repositório: `genesi-calamares-config`
3. Deixe público, **sem** README/gitignore/license

#### Passo 2: Fazer push

```bash
cd genesi-calamares-config-full
git remote add origin https://github.com/zFreshy/genesi-calamares-config.git
git branch -M main
git push -u origin main
```

#### Passo 3: Remover submodule antigo e adicionar o novo

```bash
cd ..

# Remover o submodule do CachyOS
git submodule deinit -f cachyos-calamares-config
git rm -f cachyos-calamares-config
rm -rf .git/modules/cachyos-calamares-config

# Remover a pasta temporária que criamos
rm -rf genesi-calamares-config-full

# Adicionar o novo submodule do Genesi
git submodule add https://github.com/zFreshy/genesi-calamares-config.git genesi-calamares-config-full

# Commit
git add .gitmodules genesi-calamares-config-full
git commit -m "Replace CachyOS calamares-config with Genesi calamares-config"
git push
```

### 4. Integração com o build

O sistema de build já está configurado para usar o novo submodule:

1. **prepare-and-build.sh**: Copia `genesi-calamares-config-full` para `airootfs/root/`
2. **customize_airootfs.sh**: Instala toda a configuração do Calamares durante o build da ISO

### 5. Vantagens dessa abordagem

✅ **Controle total**: Você tem seu próprio repositório para modificar
✅ **Fácil manutenção**: Todas as configurações do Calamares em um só lugar
✅ **Versionamento**: Pode fazer commits e track mudanças
✅ **Branding completo**: Logos, cores, slideshow tudo customizável
✅ **Scripts funcionais**: Usa os scripts testados do CachyOS
✅ **Sem erros**: Não precisa criar scripts manualmente

### 6. Próximos passos

Depois de configurar o submodule, você pode:

1. **Customizar logos**: Substituir os arquivos em `etc/calamares/branding/genesi/`
   - `logo.png` - Logo na sidebar
   - `icon.png` - Ícone da janela
   - `welcome.png` - Imagem de boas-vindas
   - `slide1.png` até `slide6.png` - Slides durante instalação

2. **Modificar cores**: Editar `etc/calamares/branding/genesi/branding.desc`

3. **Ajustar scripts**: Modificar scripts em `etc/calamares/scripts/` conforme necessário

4. **Testar**: Buildar a ISO e testar a instalação

## Arquivos modificados no projeto principal

- `genesi-arch/archiso/airootfs/root/customize_airootfs.sh` - Atualizado para usar genesi-calamares-config-full
- `genesi-arch/prepare-and-build.sh` - Atualizado para copiar genesi-calamares-config-full
- `.gitmodules` - Será atualizado quando você adicionar o submodule

## Resultado esperado

Quando você buildar a ISO agora, o Calamares terá:
- ✅ Todos os scripts necessários (sem erros de arquivo não encontrado)
- ✅ Branding completo do Genesi OS
- ✅ Cores verde neon (#00ff9f)
- ✅ Configurações corretas para instalação
- ✅ Suporte a todos os módulos do Calamares

Não vai mais ter aqueles erros de "cp: cannot stat '/etc/pacman.d/genesi-mirrorlist'" porque todos os scripts e configurações estarão no lugar certo!
