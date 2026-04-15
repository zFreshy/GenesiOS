import React, { useEffect, useState } from 'react';
import { X, Minus, Square } from 'lucide-react';
import { getCurrentWindow } from '@tauri-apps/api/window';
import { useTheme } from './ThemeContext';
import { motion } from 'framer-motion';

export default function WindowFrame({ children, title, icon: Icon }: { children: React.ReactNode, title: string, icon?: any }) {
  const { theme } = useTheme();
  const appWindow = getCurrentWindow();
  const [isMaximized, setIsMaximized] = useState(false);

  useEffect(() => {
    let unlisten: () => void;
    
    const checkMaximized = async () => {
      setIsMaximized(await appWindow.isMaximized());
    };
    
    checkMaximized();

    appWindow.onResized(() => {
      checkMaximized();
    }).then(unlistenFn => unlisten = unlistenFn);

    return () => {
      if (unlisten) unlisten();
    };
  }, [appWindow]);

  return (
    <motion.div 
      initial={{ opacity: 0, scale: 0.95, y: 10 }}
      animate={{ opacity: 1, scale: 1, y: 0 }}
      transition={{ duration: 0.2, ease: [0.16, 1, 0.3, 1] }}
      className={`flex flex-col w-full h-full overflow-hidden border shadow-2xl ${isMaximized ? 'rounded-none border-none' : 'rounded-xl'} ${theme === 'light' ? 'border-black/20 bg-[#f5f5f5]' : 'border-white/20 bg-[#1e1e1e]'}`}
    >
      
      {/* Custom Resize Handles - Bordas invisíveis para o Tauri redimensionar */}
      {!isMaximized && (
        <>
          <div className="absolute top-0 left-0 right-0 h-1 cursor-n-resize z-50" onPointerDown={() => (appWindow as any).startResizing('top')} />
          <div className="absolute bottom-0 left-0 right-0 h-1 cursor-s-resize z-50" onPointerDown={() => (appWindow as any).startResizing('bottom')} />
          <div className="absolute top-0 left-0 bottom-0 w-1 cursor-w-resize z-50" onPointerDown={() => (appWindow as any).startResizing('left')} />
          <div className="absolute top-0 right-0 bottom-0 w-1 cursor-e-resize z-50" onPointerDown={() => (appWindow as any).startResizing('right')} />
          <div className="absolute top-0 left-0 w-2 h-2 cursor-nw-resize z-50" onPointerDown={() => (appWindow as any).startResizing('topLeft')} />
          <div className="absolute top-0 right-0 w-2 h-2 cursor-ne-resize z-50" onPointerDown={() => (appWindow as any).startResizing('topRight')} />
          <div className="absolute bottom-0 left-0 w-2 h-2 cursor-sw-resize z-50" onPointerDown={() => (appWindow as any).startResizing('bottomLeft')} />
          <div className="absolute bottom-0 right-0 w-2 h-2 cursor-se-resize z-50" onPointerDown={() => (appWindow as any).startResizing('bottomRight')} />
        </>
      )}

      {/* Titlebar (Custom Client-Side Decoration) */}
      <div 
        className={`h-10 flex items-center justify-between px-3 shrink-0 text-white select-none ${theme === 'light' ? 'bg-[#e5e5e5] text-black border-b border-black/10' : 'bg-[#252525] border-b border-white/5'}`}
        onPointerDown={(e) => {
          // Inicia o arrasto apenas se clicar no fundo da barra (não nos botões)
          if (e.buttons === 1 && e.target === e.currentTarget) {
            appWindow.startDragging();
          }
        }}
        onDoubleClick={(e) => {
          if (e.target === e.currentTarget) {
            appWindow.toggleMaximize();
          }
        }}
      >
        <div className="flex gap-2 pl-1 pointer-events-auto">
          <div className="w-3.5 h-3.5 rounded-full bg-[#ff5f56] cursor-pointer hover:bg-red-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.close().catch(console.error); }}><X size={10} className="opacity-0 group-hover:opacity-100 text-black/70"/></div>
          <div className="w-3.5 h-3.5 rounded-full bg-[#ffbd2e] cursor-pointer hover:bg-yellow-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.minimize().catch(console.error); }}><Minus size={10} className="opacity-0 group-hover:opacity-100 text-black/70"/></div>
          <div className="w-3.5 h-3.5 rounded-full bg-[#27c93f] cursor-pointer hover:bg-green-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.toggleMaximize().catch(console.error); }}>
            {isMaximized ? <Minus size={10} className="opacity-0 group-hover:opacity-100 text-black/70"/> : <Square size={8} className="opacity-0 group-hover:opacity-100 text-black/70"/>}
          </div>
        </div>
        
        <div className="flex items-center gap-2 pointer-events-none absolute left-1/2 -translate-x-1/2">
          {Icon && <Icon size={14} className={theme === 'light' ? 'opacity-70 text-black' : 'opacity-80 text-white'} />}
          <span className={`text-xs font-semibold ${theme === 'light' ? 'text-black/80' : 'text-white/80'}`}>{title}</span>
        </div>

        <div className="w-12"></div> {/* Spacer for center alignment */}
      </div>
      
      {/* Content */}
      <div className="flex-1 overflow-hidden relative pointer-events-auto">
        {children}
      </div>
    </motion.div>
  );
}
