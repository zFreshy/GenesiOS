import React, { useState, useEffect } from 'react';
import { Globe, X, Minus, Maximize2, ExternalLink, Chrome } from 'lucide-react';
import { useTheme } from './ThemeContext';
import { invoke } from '@tauri-apps/api/core';

/**
 * WaylandBrowserApp - Lança navegadores nativos via Wayland
 * 
 * Este componente NÃO cria janelas WebView separadas.
 * Em vez disso, ele lança o navegador nativo (Chrome/Firefox) via Wayland,
 * e o Window Manager (genesi-wm) captura e gerencia a janela.
 * 
 * Quando bootar de uma ISO, o navegador vai rodar como um processo separado
 * mas será gerenciado pelo WM como qualquer outra janela do OS.
 */
const WaylandBrowserApp = ({ 
  onClose, 
  onMinimize, 
  onMaximize 
}: { 
  onClose?: () => void, 
  onMinimize?: () => void, 
  onMaximize?: () => void 
}) => {
  const { theme } = useTheme();
  const [browserLaunched, setBrowserLaunched] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    // Lança o navegador automaticamente quando o componente monta
    launchBrowser();
  }, []);

  const launchBrowser = async () => {
    try {
      setError(null);
      setBrowserLaunched(false);
      
      // Chama o comando Rust que lança o navegador via Wayland
      // O WM vai capturar a janela e gerenciá-la
      await invoke('launch_browser_wayland');
      
      setBrowserLaunched(true);
      
      // Fecha este placeholder após 2 segundos
      // A janela real do navegador já está sendo gerenciada pelo WM
      setTimeout(() => {
        onClose?.();
      }, 2000);
      
    } catch (e) {
      console.error('Failed to launch browser:', e);
      setError(String(e));
    }
  };

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f3f3f3] text-black' : 'bg-[#202020] text-white'}`}>
      {/* Barra superior integrada (macOS style) */}
      <div className={`flex items-center px-4 py-3 border-b ${theme === 'light' ? 'bg-[#f3f3f3] border-black/10' : 'bg-[#202020] border-white/10'}`}>
        {/* Window Control Buttons */}
        <div className="flex gap-2">
          <div 
            className="w-3.5 h-3.5 rounded-full bg-[#ff5f56] cursor-pointer hover:bg-red-400 transition-colors flex items-center justify-center group" 
            onClick={(e) => { e.preventDefault(); onClose?.(); }}
          >
            <X size={10} className="opacity-0 group-hover:opacity-100 text-black"/>
          </div>
          <div 
            className="w-3.5 h-3.5 rounded-full bg-[#ffbd2e] cursor-pointer hover:bg-yellow-400 transition-colors flex items-center justify-center group" 
            onClick={(e) => { e.preventDefault(); onMinimize?.(); }}
          >
            <Minus size={10} className="opacity-0 group-hover:opacity-100 text-black"/>
          </div>
          <div 
            className="w-3.5 h-3.5 rounded-full bg-[#27c93f] cursor-pointer hover:bg-green-400 transition-colors flex items-center justify-center group" 
            onClick={(e) => { e.preventDefault(); onMaximize?.(); }}
          >
            <Maximize2 size={8} className="opacity-0 group-hover:opacity-100 text-black"/>
          </div>
        </div>

        <div className="flex-1 flex items-center justify-center gap-2">
          <Chrome size={16} className="text-blue-500" />
          <span className="text-sm font-medium">Genesi Browser</span>
        </div>

        <div className="w-[60px]"></div>
      </div>

      {/* Content Area */}
      <div className="flex-1 flex flex-col items-center justify-center p-8 text-center">
        {!browserLaunched && !error && (
          <>
            <div className="animate-spin rounded-full h-16 w-16 border-b-2 border-blue-500 mb-4"></div>
            <h2 className="text-2xl font-semibold mb-2">Abrindo Navegador...</h2>
            <p className={`mb-4 max-w-md ${theme === 'light' ? 'text-gray-600' : 'text-gray-400'}`}>
              O navegador está sendo iniciado via Wayland.
            </p>
          </>
        )}

        {browserLaunched && !error && (
          <>
            <ExternalLink size={64} className="text-green-500 mb-4" />
            <h2 className="text-2xl font-semibold mb-2">Navegador Aberto!</h2>
            <p className={`mb-4 max-w-md ${theme === 'light' ? 'text-gray-600' : 'text-gray-400'}`}>
              O navegador está rodando como uma janela separada gerenciada pelo Window Manager.
            </p>
            <p className={`text-sm ${theme === 'light' ? 'text-gray-500' : 'text-gray-500'}`}>
              Esta janela será fechada automaticamente...
            </p>
          </>
        )}

        {error && (
          <>
            <X size={64} className="text-red-500 mb-4" />
            <h2 className="text-2xl font-semibold mb-2">Erro ao Abrir Navegador</h2>
            <p className={`mb-4 max-w-md ${theme === 'light' ? 'text-gray-600' : 'text-gray-400'}`}>
              {error}
            </p>
            <button
              onClick={launchBrowser}
              className="px-6 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded-lg transition-colors"
            >
              Tentar Novamente
            </button>
          </>
        )}

        <div className={`mt-8 p-4 rounded-lg max-w-2xl ${theme === 'light' ? 'bg-blue-50 text-blue-900' : 'bg-blue-900/20 text-blue-300'}`}>
          <h3 className="font-semibold mb-2">Como funciona:</h3>
          <ul className="text-sm text-left space-y-1">
            <li>• <strong>Desenvolvimento (WSL/Windows):</strong> O navegador abre como janela separada do Windows</li>
            <li>• <strong>Sistema Real (ISO bootável):</strong> O navegador roda dentro do Genesi OS, gerenciado pelo Window Manager</li>
            <li>• <strong>Uma barra apenas:</strong> O navegador usa sua própria barra de título (sem duplicação)</li>
          </ul>
        </div>
      </div>
    </div>
  );
};

export default WaylandBrowserApp;
