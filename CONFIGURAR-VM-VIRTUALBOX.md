# 🖥️ Como Configurar a VM no VirtualBox Corretamente

## Configurações Recomendadas para Genesi OS

### 1. Criar Nova VM

1. **Abra VirtualBox** → Click "New"

2. **Name and Operating System**:
   - Name: `GenesiOS`
   - Type: `Linux`
   - Version: `Ubuntu (64-bit)`
   - Click "Next"

3. **Memory Size**:
   - RAM: `4096 MB` (4 GB) - **MÍNIMO**
   - Recomendado: `8192 MB` (8 GB) se tiver disponível
   - Click "Next"

4. **Hard Disk**:
   - Select "Create a virtual hard disk now"
   - Click "Create"

5. **Hard Disk File Type**:
   - Select "VDI (VirtualBox Disk Image)"
   - Click "Next"

6. **Storage on Physical Hard Disk**:
   - Select "Dynamically allocated"
   - Click "Next"

7. **File Location and Size**:
   - Size: `20 GB` (mínimo)
   - Click "Create"

### 2. Configurar a VM (IMPORTANTE!)

Antes de iniciar, ajuste estas configurações:

#### A. System Settings

1. **Settings → System → Motherboard**:
   - ✅ Boot Order: Optical primeiro, depois Hard Disk
   - ✅ Chipset: `ICH9`
   - ❌ **DESMARQUE** "Enable EFI" (pode causar problemas)
   - Base Memory: `4096 MB` ou mais

2. **Settings → System → Processor**:
   - CPUs: `4` (ou metade dos seus cores)
   - ✅ Enable PAE/NX

3. **Settings → System → Acceleration**:
   - Paravirtualization Interface: **`Default`** ou **`None`**
   - ❌ **NÃO USE KVM** (causa o erro que você teve)
   - ✅ Enable VT-x/AMD-V
   - ✅ Enable Nested Paging

#### B. Display Settings

**Settings → Display → Screen**:
- Video Memory: **`128 MB`** (MÍNIMO)
- Graphics Controller: `VMSVGA` ou `VBoxVGA`
- ❌ **DESMARQUE** "Enable 3D Acceleration" (causa problemas com Wayland)
- Scale Factor: `100%`

#### C. Storage Settings

**Settings → Storage**:
1. Click no ícone do CD (Controller: IDE)
2. Click no ícone do disco à direita
3. "Choose a disk file..."
4. Selecione sua ISO: `GenesiOS-YYYYMMDD.iso`
5. ✅ Marque "Live CD/DVD"

#### D. Network Settings (Opcional)

**Settings → Network → Adapter 1**:
- ✅ Enable Network Adapter
- Attached to: `NAT` (para ter internet)

### 3. Iniciar a VM

1. **Selecione a VM** "GenesiOS"
2. Click **"Start"** (botão verde)
3. **Aguarde o GRUB** aparecer
4. Selecione **"Genesi OS"** (primeira opção)
5. Aguarde o boot (~30-60 segundos)

### 4. O Que Esperar

#### Boot Sequence:
1. **GRUB Menu** (5 segundos)
2. **Linux Kernel** carregando
3. **Initramfs** montando sistema
4. **Live CD** iniciando
5. **Autologin** como usuário `genesi`
6. **Sway** iniciando (Wayland compositor)
7. **Desktop GTK4** aparecendo

#### Se Tudo Funcionar:
- ✅ Wallpaper aparece
- ✅ Dock na parte inferior
- ✅ Ícones arrastáveis na área de trabalho
- ✅ Mouse funciona
- ✅ Apps abrem com double-click

### 5. Troubleshooting

#### Se a VM não bootar:

**Opção 1: Safe Mode**
- No GRUB, selecione "Genesi OS (Safe Mode)"
- Isso desabilita aceleração gráfica

**Opção 2: Debug Mode**
- No GRUB, selecione "Genesi OS (Debug Mode)"
- Você verá logs detalhados do boot

**Opção 3: Failsafe Mode**
- No GRUB, selecione "Genesi OS (Failsafe)"
- Modo mais seguro, sem aceleração

#### Se aparecer tela preta:

1. **Pressione Ctrl+Alt+F2** para ir ao terminal
2. **Login**: `genesi` / Senha: `genesi`
3. **Veja os logs**:
   ```bash
   cat /tmp/genesi-startup.log
   ```

#### Se o erro VERR_UNRESOLVED_ERROR persistir:

1. **Delete a VM** completamente
2. **Recrie** seguindo EXATAMENTE os passos acima
3. **IMPORTANTE**: Paravirtualization = `Default` ou `None` (NÃO KVM!)
4. **IMPORTANTE**: Desabilite 3D Acceleration

### 6. Configurações Alternativas (Se Não Funcionar)

#### Tentar com EFI:
1. Settings → System → Motherboard
2. ✅ Enable EFI (special OSes only)
3. Tente bootar novamente

#### Aumentar Recursos:
- RAM: 8 GB
- CPUs: 6 cores
- Video Memory: 256 MB

#### Mudar Graphics Controller:
- Tente `VBoxVGA` em vez de `VMSVGA`
- Ou vice-versa

## 🎯 Checklist Rápido

Antes de iniciar a VM, verifique:

- [ ] RAM: 4 GB ou mais
- [ ] CPUs: 4 ou mais
- [ ] Video Memory: 128 MB ou mais
- [ ] Paravirtualization: Default ou None (NÃO KVM)
- [ ] 3D Acceleration: DESABILITADO
- [ ] ISO montada no drive de CD
- [ ] Boot Order: Optical primeiro

## 📞 Se Ainda Não Funcionar

Me envie:
1. Screenshot do erro completo
2. Tamanho da ISO gerada (`ls -lh GenesiOS-*.iso`)
3. Configurações da VM (screenshot)
4. Logs se conseguir acessar o terminal (Ctrl+Alt+F2)
