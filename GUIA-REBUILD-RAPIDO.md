# Guia de Rebuild Rápido da ISO

## Quando Usar

Use o rebuild rápido quando você já deu `build-iso.sh` uma vez e só quer recompilar o código do Genesi (WM + Desktop) sem refazer toda a ISO.

## Pré-requisitos

- Já ter executado `build-iso.sh` pelo menos uma vez
- Pasta `~/genesi-iso-build` deve existir
- Chroot deve estar configurado

## Comando Rápido

```bash
cd ~/GenesiOS
bash rebuild-iso.sh
```

## O Que o Rebuild Faz

1. **Recompila apenas o código Genesi**:
   - Genesi WM (Window Manager)
   - Genesi Desktop (Interface Tauri)

2. **Copia binários para o chroot**

3. **Recria a ISO** com os novos binários

4. **Não reinstala**:
   - Pacotes do sistema
   - Dependências
   - Configurações base

## Tempo Estimado

- **Build completo**: 15-30 minutos
- **Rebuild rápido**: 3-5 minutos

## Quando Fazer Build Completo

Faça `build-iso.sh` completo quando:

- Primeira vez criando a ISO
- Mudou dependências (Cargo.toml, package.json)
- Mudou configuração do sistema
- Adicionou novos pacotes
- Chroot foi deletado ou corrompido

## Quando Fazer Rebuild Rápido

Faça `rebuild-iso.sh` quando:

- Mudou código Rust do WM
- Mudou código TypeScript/React do Desktop
- Mudou código Rust do Tauri backend
- Quer testar mudanças rapidamente

## Estrutura de Pastas

```
~/genesi-iso-build/
├── chroot/                    # Sistema base (preservado no rebuild)
│   ├── home/genesi/GenesiOS/
│   │   └── genesi-desktop/
│   │       ├── genesi-wm/target/release/genesi-wm      ← Atualizado
│   │       └── src-tauri/target/release/genesi-desktop ← Atualizado
│   └── usr/local/bin/start-genesi.sh
├── iso/                       # ISO gerada
└── genesi-os.iso             # Arquivo final
```

## Troubleshooting

### Erro: "Pasta ~/genesi-iso-build não existe"

**Solução**: Execute `build-iso.sh` primeiro:
```bash
cd ~/GenesiOS
bash build-iso.sh
```

### Erro: "Falha na compilação"

**Solução**: Verifique os logs:
```bash
sudo chroot ~/genesi-iso-build/chroot cat /tmp/wm-build.log
sudo chroot ~/genesi-iso-build/chroot cat /tmp/tauri-build.log
```

### Erro: "Permission denied"

**Solução**: Use sudo:
```bash
sudo bash rebuild-iso.sh
```

### ISO não funciona após rebuild

**Solução**: Faça build completo:
```bash
cd ~/GenesiOS
bash build-iso.sh
```

## Testando a ISO

### No VirtualBox/VMware:
1. Crie VM com 4GB RAM, 40GB disco
2. Anexe `genesi-os.iso` como CD
3. Boot da ISO
4. Sistema deve iniciar automaticamente

### Verificar Logs:
```bash
# Dentro da ISO (Ctrl+Alt+F2)
cat /tmp/genesi-startup.log
```

## Comandos Úteis

### Ver espaço usado:
```bash
du -sh ~/genesi-iso-build/
```

### Limpar builds antigos:
```bash
sudo rm -rf ~/genesi-iso-build/chroot/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target
sudo rm -rf ~/genesi-iso-build/chroot/home/genesi/GenesiOS/genesi-desktop/src-tauri/target
```

### Recriar do zero:
```bash
sudo rm -rf ~/genesi-iso-build
bash build-iso.sh
```
