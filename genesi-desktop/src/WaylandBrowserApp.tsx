import { useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';

/**
 * WaylandBrowserApp - Lança navegadores nativos via Wayland
 * 
 * Este componente NÃO mostra nada na tela.
 * Apenas lança o navegador nativo e fecha imediatamente.
 */
const WaylandBrowserApp = ({ 
  onClose
}: { 
  onClose?: () => void
}) => {
  useEffect(() => {
    const launchAndClose = async () => {
      try {
        // Lança o navegador
        await invoke('launch_browser_wayland');
        console.log('✅ Navegador lançado com sucesso');
      } catch (e) {
        console.error('❌ Erro ao lançar navegador:', e);
      } finally {
        // Fecha esta janela placeholder imediatamente
        onClose?.();
      }
    };

    launchAndClose();
  }, [onClose]);

  // Não renderiza nada (ou renderiza invisível)
  return null;
};

export default WaylandBrowserApp;
