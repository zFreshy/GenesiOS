import React, { useState } from 'react';
import { Globe, ArrowRight, RotateCw, X, Minus, Maximize2, ArrowLeft, Home } from 'lucide-react';
import { useTheme } from './ThemeContext';

const BrowserApp = ({ onClose, onMinimize, onMaximize }: { onClose?: () => void, onMinimize?: () => void, onMaximize?: () => void }) => {
  const { theme } = useTheme();
  const [urlInput, setUrlInput] = useState('https://www.google.com');
  const [currentUrl, setCurrentUrl] = useState('https://www.google.com');
  const [isLoading, setIsLoading] = useState(false);
  const [history, setHistory] = useState<string[]>(['https://www.google.com']);
  const [historyIndex, setHistoryIndex] = useState(0);

  const handleNavigate = (e: React.FormEvent) => {
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
    
    setIsLoading(true);
    setCurrentUrl(finalUrl);
    setUrlInput(finalUrl);
    
    // Adiciona ao histórico
    const newHistory = history.slice(0, historyIndex + 1);
    newHistory.push(finalUrl);
    setHistory(newHistory);
    setHistoryIndex(newHistory.length - 1);
  };

  const handleGoBack = () => {
    if (historyIndex > 0) {
      const newIndex = historyIndex - 1;
      setHistoryIndex(newIndex);
      const url = history[newIndex];
      setCurrentUrl(url);
      setUrlInput(url);
    }
  };

  const handleGoForward = () => {
    if (historyIndex < history.length - 1) {
      const newIndex = historyIndex + 1;
      setHistoryIndex(newIndex);
      const url = history[newIndex];
      setCurrentUrl(url);
      setUrlInput(url);
    }
  };

  const handleGoHome = () => {
    const homeUrl = 'https://www.google.com';
    setUrlInput(homeUrl);
    setCurrentUrl(homeUrl);
    setHistory([...history, homeUrl]);
    setHistoryIndex(history.length);
  };

  const handleReload = () => {
    setIsLoading(true);
    // Trick to force iframe reload
    const temp = currentUrl;
    setCurrentUrl('');
    setTimeout(() => {
      setCurrentUrl(temp);
      setIsLoading(false);
    }, 10);
  };

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f3f3f3] text-black' : 'bg-[#202020] text-white'}`}>
      {/* Navigation Bar with Window Controls */}
      <form onSubmit={handleNavigate} className={`flex items-center px-4 py-2 gap-3 border-b ${theme === 'light' ? 'bg-[#f3f3f3] border-black/10' : 'bg-[#202020] border-white/10'}`}>
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

        {/* Navigation Buttons */}
        <div className="flex gap-1">
          <button 
            type="button" 
            onClick={handleGoBack} 
            disabled={historyIndex === 0}
            className={`p-1.5 rounded-md transition-colors ${historyIndex === 0 ? 'opacity-30 cursor-not-allowed' : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10')}`}
          >
            <ArrowLeft size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
          </button>
          <button 
            type="button" 
            onClick={handleGoForward} 
            disabled={historyIndex === history.length - 1}
            className={`p-1.5 rounded-md transition-colors ${historyIndex === history.length - 1 ? 'opacity-30 cursor-not-allowed' : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10')}`}
          >
            <ArrowRight size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
          </button>
          <button type="button" onClick={handleReload} className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
            <RotateCw size={16} className={`${isLoading ? 'animate-spin text-blue-500' : (theme === 'light' ? 'text-gray-600' : 'text-gray-300')}`} />
          </button>
          <button type="button" onClick={handleGoHome} className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
            <Home size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
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

      {/* Web View - Navegador Real via iframe */}
      <div className="flex-1 relative bg-white">
        {currentUrl ? (
          <iframe 
            src={currentUrl} 
            className="w-full h-full border-none bg-white" 
            title="Genesi Browser"
            sandbox="allow-same-origin allow-scripts allow-forms allow-popups allow-downloads allow-modals allow-popups-to-escape-sandbox"
            onLoad={() => setIsLoading(false)}
            onError={() => setIsLoading(false)}
            allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
          />
        ) : (
          <div className="w-full h-full flex items-center justify-center text-gray-400">
            Carregando...
          </div>
        )}
        
        {/* Loading Overlay */}
        {isLoading && (
          <div className="absolute inset-0 bg-white/50 flex items-center justify-center">
            <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-blue-500"></div>
          </div>
        )}
      </div>
    </div>
  );
};

export default BrowserApp;
