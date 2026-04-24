import React, { useState } from 'react';
import { Globe, ArrowRight, RotateCw, X, Minus, Maximize2 } from 'lucide-react';
import { useTheme } from './ThemeContext';

const BrowserApp = ({ onClose, onMinimize, onMaximize }: { onClose?: () => void, onMinimize?: () => void, onMaximize?: () => void }) => {
  const { theme } = useTheme();
  const [urlInput, setUrlInput] = useState('https://duckduckgo.com');
  const [currentUrl, setCurrentUrl] = useState('https://duckduckgo.com');
  const [isLoading, setIsLoading] = useState(false);

  const handleNavigate = (e: React.FormEvent) => {
    e.preventDefault();
    let finalUrl = urlInput;
    
    // Auto-complete http protocol if missing and not localhost
    if (!finalUrl.startsWith('http://') && !finalUrl.startsWith('https://')) {
      finalUrl = 'https://' + finalUrl;
    }
    
    setIsLoading(true);
    setCurrentUrl(finalUrl);
    setUrlInput(finalUrl);
  };

  const handleReload = () => {
    setIsLoading(true);
    // Trick to force iframe reload
    const temp = currentUrl;
    setCurrentUrl('');
    setTimeout(() => setCurrentUrl(temp), 10);
  };

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f3f3f3] text-black' : 'bg-[#202020] text-white'}`}>
      {/* Navegation Bar with Window Controls */}
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

        <div className="flex gap-1">
          <button type="button" onClick={handleReload} className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
            <RotateCw size={16} className={`${isLoading ? 'animate-spin text-blue-500' : (theme === 'light' ? 'text-gray-600' : 'text-gray-300')}`} />
          </button>
        </div>

        <div className={`flex-1 flex items-center gap-2 px-3 py-1.5 rounded-full border ${theme === 'light' ? 'bg-white border-black/10' : 'bg-black/40 border-white/10 focus-within:border-blue-500/50'}`}>
          <Globe size={14} className={theme === 'light' ? 'text-gray-400' : 'text-gray-500'} />
          <input
            type="text"
            value={urlInput}
            onChange={(e) => setUrlInput(e.target.value)}
            className="flex-1 bg-transparent outline-none text-sm w-full"
            placeholder="Search or enter web address"
          />
        </div>

        <button type="submit" className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
          <ArrowRight size={16} className={theme === 'light' ? 'text-gray-600' : 'text-gray-300'} />
        </button>
      </form>

      {/* Web View (Iframe) */}
      <div className="flex-1 relative bg-white">
        {currentUrl ? (
          <iframe 
            src={currentUrl} 
            className="w-full h-full border-none bg-white" 
            title="Genesi Browser"
            sandbox="allow-same-origin allow-scripts allow-forms allow-popups"
            onLoad={() => setIsLoading(false)}
            onError={() => setIsLoading(false)}
          />
        ) : (
          <div className="w-full h-full flex items-center justify-center text-gray-400">
            Carregando...
          </div>
        )}
      </div>
    </div>
  );
};

export default BrowserApp;
