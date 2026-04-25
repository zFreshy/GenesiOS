import { useState, useEffect } from 'react';
import { Globe, ArrowRight, RotateCw, X, Minus, Maximize2, Home, Plus, ExternalLink } from 'lucide-react';
import { useTheme } from './ThemeContext';
import { invoke } from '@tauri-apps/api/core';

interface Tab {
  id: string;
  url: string;
  title: string;
  windowLabel?: string;
}

const NativeBrowserApp = ({ onClose, onMinimize, onMaximize }: { onClose?: () => void, onMinimize?: () => void, onMaximize?: () => void }) => {
  const { theme } = useTheme();
  const [tabs, setTabs] = useState<Tab[]>([
    { id: '1', url: 'https://www.google.com', title: 'Google' }
  ]);
  const [activeTabId, setActiveTabId] = useState('1');
  const [urlInput, setUrlInput] = useState('https://www.google.com');
  const [isLoading, setIsLoading] = useState(false);

  const activeTab = tabs.find(t => t.id === activeTabId) || tabs[0];

  useEffect(() => {
    // Cria a primeira janela de navegador
    if (activeTab && !activeTab.windowLabel) {
      createBrowserWindow(activeTab.id, activeTab.url, activeTab.title);
    }
  }, []);

  const createBrowserWindow = async (tabId: string, url: string, title: string) => {
    try {
      setIsLoading(true);
      const windowLabel = await invoke<string>('create_browser_window', {
        url,
        title: `Genesi Browser - ${title}`
      });

      setTabs(prev => prev.map(t => 
        t.id === tabId ? { ...t, windowLabel } : t
      ));

      console.log('Browser window created:', windowLabel);
    } catch (e) {
      console.error('Failed to create browser window:', e);
      alert(`Erro ao criar janela do navegador: ${e}`);
    } finally {
      setIsLoading(false);
    }
  };

  const handleNavigate = async (e: React.FormEvent) => {
    e.preventDefault();
    let finalUrl = urlInput;
    
    // Auto-complete http protocol if missing
    if (!finalUrl.startsWith('http://') && !finalUrl.startsWith('https://')) {
      // Se parece com domínio, adiciona https
      if (finalUrl.includes('.') && !finalUrl.includes(' ')) {
        finalUrl = 'https://' + finalUrl;
      } else {
        // Senão, busca no Google
        finalUrl = `https://www.google.com/search?q=${encodeURIComponent(finalUrl)}`;
      }
    }
    
    // Atualiza a tab atual
    setTabs(prev => prev.map(t => 
      t.id === activeTabId ? { ...t, url: finalUrl, title: new URL(finalUrl).hostname } : t
    ));

    // Cria nova janela com a URL
    await createBrowserWindow(activeTabId, finalUrl, new URL(finalUrl).hostname);
  };

  const handleNewTab = () => {
    const newTab: Tab = {
      id: Date.now().toString(),
      url: 'https://www.google.com',
      title: 'Nova aba'
    };
    setTabs(prev => [...prev, newTab]);
    setActiveTabId(newTab.id);
    setUrlInput(newTab.url);
    createBrowserWindow(newTab.id, newTab.url, newTab.title);
  };

  const handleCloseTab = (tabId: string, e: React.MouseEvent) => {
    e.stopPropagation();
    
    if (tabs.length === 1) {
      // Se for a última aba, fecha o navegador
      onClose?.();
      return;
    }

    const tabIndex = tabs.findIndex(t => t.id === tabId);
    const newTabs = tabs.filter(t => t.id !== tabId);
    setTabs(newTabs);

    // Se fechou a aba ativa, ativa a próxima ou anterior
    if (tabId === activeTabId) {
      const newActiveTab = newTabs[Math.min(tabIndex, newTabs.length - 1)];
      setActiveTabId(newActiveTab.id);
      setUrlInput(newActiveTab.url);
    }
  };

  const handleGoHome = () => {
    setUrlInput('https://www.google.com');
    handleNavigate({ preventDefault: () => {} } as React.FormEvent);
  };

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f3f3f3] text-black' : 'bg-[#202020] text-white'}`}>
      {/* Navigation Bar with Window Controls */}
      <div className={`flex flex-col border-b ${theme === 'light' ? 'bg-[#f3f3f3] border-black/10' : 'bg-[#202020] border-white/10'}`}>
        
        {/* Top Bar - Window Controls + Tabs */}
        <div className="flex items-center px-4 py-2 gap-3">
          {/* Window Control Buttons (macOS style) */}
          <div className="flex gap-2 pr-3 border-r border-white/10">
            <div className="w-3.5 h-3.5 rounded-full bg-[#ff5f56] cursor-pointer hover:bg-red-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.preventDefault(); onClose?.(); }}>
              <X size={10} className="opacity-0 group-hover:opacity-100 text-black"/>
            </div>
            <div className="w-3.5 h-3.5 rounded-full bg-[#ffbd2e] cursor-pointer hover:bg-yellow-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.preventDefault(); onMinimize?.(); }}>
              <Minus size={10} className="opacity-0 group-hover:opacity-100 text-black"/>
            </div>
            <div className="w-3.5 h-3.5 rounded-full bg-[#27c93f] cursor-pointer hover:bg-green-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.preventDefault(); onMaximize?.(); }}>
              <Maximize2 size={8} className="opacity-0 group-hover:opacity-100 text-black"/>
            </div>
          </div>

          {/* Tabs */}
          <div className="flex-1 flex items-center gap-1 overflow-x-auto">
            {tabs.map(tab => (
              <div
                key={tab.id}
                onClick={() => {
                  setActiveTabId(tab.id);
                  setUrlInput(tab.url);
                }}
                className={`flex items-center gap-2 px-3 py-1.5 rounded-t-lg cursor-pointer transition-colors min-w-[150px] max-w-[200px] group ${
                  tab.id === activeTabId 
                    ? (theme === 'light' ? 'bg-white' : 'bg-[#2d2d2d]')
                    : (theme === 'light' ? 'bg-gray-200 hover:bg-gray-300' : 'bg-[#1a1a1a] hover:bg-[#252525]')
                }`}
              >
                <Globe size={14} className="shrink-0" />
                <span className="text-xs truncate flex-1">{tab.title}</span>
                {tabs.length > 1 && (
                  <X 
                    size={14} 
                    className="shrink-0 opacity-0 group-hover:opacity-100 hover:bg-red-500/20 rounded" 
                    onClick={(e) => handleCloseTab(tab.id, e)}
                  />
                )}
              </div>
            ))}
            
            {/* New Tab Button */}
            <button
              onClick={handleNewTab}
              className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
              title="Nova aba"
            >
              <Plus size={16} />
            </button>
          </div>
        </div>

        {/* Address Bar */}
        <form onSubmit={handleNavigate} className="flex items-center px-4 py-2 gap-3">
          {/* Navigation Buttons */}
          <div className="flex gap-1">
            <button type="button" onClick={handleGoHome} className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
              <Home size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
            </button>
            <button type="button" onClick={() => handleNavigate({ preventDefault: () => {} } as React.FormEvent)} className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
              <RotateCw size={16} className={`${isLoading ? 'animate-spin text-blue-500' : (theme === 'light' ? 'text-gray-600' : 'text-gray-300')}`} />
            </button>
          </div>

          {/* URL Bar */}
          <div className={`flex-1 flex items-center gap-2 px-3 py-1.5 rounded-full border ${theme === 'light' ? 'bg-white border-black/10' : 'bg-black/40 border-white/10 focus-within:border-blue-500/50'}`}>
            <Globe size={14} className={theme === 'light' ? 'text-gray-400' : 'text-gray-500'} />
            <input
              type="text"
              value={urlInput}
              onChange={(e) => setUrlInput(e.target.value)}
              className="flex-1 bg-transparent outline-none text-sm w-full"
              placeholder="Search Google or type a URL"
            />
          </div>

          <button type="submit" className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
            <ArrowRight size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
          </button>
        </form>
      </div>

      {/* Content Area - Info sobre WebView */}
      <div className="flex-1 flex flex-col items-center justify-center p-8 text-center">
        <ExternalLink size={64} className="text-blue-500 mb-4" />
        <h2 className="text-2xl font-semibold mb-2">Navegador WebView Nativo</h2>
        <p className={`mb-4 max-w-md ${theme === 'light' ? 'text-gray-600' : 'text-gray-400'}`}>
          O navegador está rodando em uma janela separada usando o motor Chromium nativo do sistema.
        </p>
        <div className={`p-4 rounded-lg ${theme === 'light' ? 'bg-blue-50 text-blue-900' : 'bg-blue-900/20 text-blue-300'}`}>
          <p className="text-sm">
            <strong>Aba ativa:</strong> {activeTab.title}
          </p>
          <p className="text-xs mt-1 opacity-70">
            {activeTab.url}
          </p>
        </div>
        
        {isLoading && (
          <div className="mt-4 flex items-center gap-2">
            <div className="animate-spin rounded-full h-4 w-4 border-b-2 border-blue-500"></div>
            <span className="text-sm">Abrindo janela do navegador...</span>
          </div>
        )}
      </div>
    </div>
  );
};

export default NativeBrowserApp;
