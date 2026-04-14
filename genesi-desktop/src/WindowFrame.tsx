import React from 'react';
import { X, Minus, Square } from 'lucide-react';
import { getCurrentWindow } from '@tauri-apps/api/window';
import { useTheme } from './ThemeContext';

export default function WindowFrame({ children, title, icon: Icon }: { children: React.ReactNode, title: string, icon?: any }) {
  const { theme } = useTheme();
  const appWindow = getCurrentWindow();

  return (
    <div className={`flex flex-col w-full h-full overflow-hidden rounded-xl border shadow-2xl ${theme === 'light' ? 'border-black/20 bg-[#f5f5f5]' : 'border-white/20 bg-[#1e1e1e]'}`}>
      
      {/* Custom Resize Handles - Bordas invisíveis para o Tauri redimensionar */}
      <div className="absolute top-0 left-0 right-0 h-1 cursor-n-resize z-50" onPointerDown={() => appWindow.startResizing('top')} />
      <div className="absolute bottom-0 left-0 right-0 h-1 cursor-s-resize z-50" onPointerDown={() => appWindow.startResizing('bottom')} />
      <div className="absolute top-0 left-0 bottom-0 w-1 cursor-w-resize z-50" onPointerDown={() => appWindow.startResizing('left')} />
      <div className="absolute top-0 right-0 bottom-0 w-1 cursor-e-resize z-50" onPointerDown={() => appWindow.startResizing('right')} />
      <div className="absolute top-0 left-0 w-2 h-2 cursor-nw-resize z-50" onPointerDown={() => appWindow.startResizing('topLeft')} />
      <div className="absolute top-0 right-0 w-2 h-2 cursor-ne-resize z-50" onPointerDown={() => appWindow.startResizing('topRight')} />
      <div className="absolute bottom-0 left-0 w-2 h-2 cursor-sw-resize z-50" onPointerDown={() => appWindow.startResizing('bottomLeft')} />
      <div className="absolute bottom-0 right-0 w-2 h-2 cursor-se-resize z-50" onPointerDown={() => appWindow.startResizing('bottomRight')} />

      {/* Titlebar (Custom Client-Side Decoration) */}
      <div 
        className="h-10 flex items-center justify-between px-3 shrink-0 bg-black/80 text-white select-none"
        data-tauri-drag-region
        onPointerDown={(e) => {
          // Inicia o arrasto apenas se clicar no fundo da barra (não nos botões)
          if (e.buttons === 1 && e.target === e.currentTarget) {
            appWindow.startDragging();
          }
        }}
        onDoubleClick={() => appWindow.toggleMaximize()}
      >
        <div className="flex gap-2 pl-1 pointer-events-auto">
          <div className="w-3.5 h-3.5 rounded-full bg-[#ff5f56] cursor-pointer hover:bg-red-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.close(); }}><X size={10} className="opacity-0 group-hover:opacity-100 text-black"/></div>
          <div className="w-3.5 h-3.5 rounded-full bg-[#ffbd2e] cursor-pointer hover:bg-yellow-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.minimize(); }}><span className="opacity-0 group-hover:opacity-100 text-black text-[10px] font-bold">-</span></div>
          <div className="w-3.5 h-3.5 rounded-full bg-[#27c93f] cursor-pointer hover:bg-green-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); appWindow.toggleMaximize(); }}><span className="opacity-0 group-hover:opacity-100 text-black text-[10px] font-bold">+</span></div>
        </div>
        
        <div className="flex items-center gap-2 pointer-events-none absolute left-1/2 -translate-x-1/2">
          {Icon && <Icon size={14} className="opacity-80" />}
          <span className="text-xs font-semibold text-white/80">{title}</span>
        </div>

        <div className="w-12"></div> {/* Spacer for center alignment */}
      </div>
      
      {/* Content */}
      <div className="flex-1 overflow-hidden relative pointer-events-auto">
        {children}
      </div>
    </div>
  );
}
