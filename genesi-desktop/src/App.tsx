import { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence, useDragControls, useAnimation } from 'framer-motion';
import { LazyStore } from '@tauri-apps/plugin-store';
import {
  Wifi, Battery, Globe, Terminal, Package, Folder, Activity, Settings, X, Play, List, Trash2, LayoutGrid, Monitor, MonitorUp
} from 'lucide-react';
import { IconBrandChrome } from '@tabler/icons-react';
import './index.css';
import StartMenu from './StartMenu';
import StartContextMenu from './StartContextMenu';
import SettingsApp from './SettingsApp';
import FileExplorer from './FileExplorer';
import TaskManager from './TaskManager';
import ControlCenter from './ControlCenter';
import { useTheme } from './ThemeContext';
import { useDisplay } from './DisplayContext';

import { Command } from '@tauri-apps/plugin-shell';
import { WebviewWindow } from '@tauri-apps/api/webviewWindow';

import ImageViewer from './ImageViewer';
import VideoPlayer from './VideoPlayer';
import TextEditor from './TextEditor';
import TerminalApp from './TerminalApp';
import BrowserApp from './BrowserApp';

// --- Relógio isolado para a Taskbar ---
const TaskbarClock = () => {
  const [time, setTime] = useState(new Date());
  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);
  return (
    <div className="flex flex-col text-right text-[10px] leading-tight opacity-80">
      <span>{time.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}</span>
      <span>{time.toLocaleDateString()}</span>
    </div>
  );
};

let globalZIndex = 10;
const appStateStore = new LazyStore('appState.json');

const DesktopIconItem = ({ 
  id, defaultIndex, primaryDisplay, getGridSnapSize, 
  iconPositions, updateIconPosition, onDoubleClick, onContextMenu,
  children, getIconClasses 
}: any) => {
  const controls = useAnimation();
  
  const step = getGridSnapSize();
  const maxRows = Math.max(1, Math.floor((primaryDisplay.logicalHeight - 60) / step));
  const maxCols = Math.max(1, Math.floor(primaryDisplay.logicalWidth / step));
  
  const defaultCol = Math.floor(defaultIndex / maxRows);
  const defaultRow = defaultIndex % maxRows;
  const pos = iconPositions[id] || { col: defaultCol, row: defaultRow };
  
  const boundedCol = Math.max(0, Math.min(pos.col, maxCols - 1));
  const boundedRow = Math.max(0, Math.min(pos.row, maxRows - 1));

  const targetX = 16 + (boundedCol * step);
  const targetY = 16 + (boundedRow * step);

  useEffect(() => {
    controls.start({ x: targetX, y: targetY, transition: { type: 'spring', bounce: 0, duration: 0.3 } });
  }, [targetX, targetY, controls]);

  return (
    <motion.div
      drag
      dragMomentum={false}
      animate={controls}
      initial={{ x: targetX, y: targetY }}
      transition={{ type: 'spring', bounce: 0, duration: 0.3 }}
      dragConstraints={{ left: 16, top: 16, right: primaryDisplay.logicalWidth - step, bottom: primaryDisplay.logicalHeight - 60 - step }}
      onDragEnd={(_, info) => {
        updateIconPosition(id, info.point.x, info.point.y, defaultIndex);
        // Force snap back even if state doesn't change
        controls.start({ x: targetX, y: targetY, transition: { type: 'spring', bounce: 0, duration: 0.3 } });
      }}
      className={`absolute flex flex-col items-center justify-center rounded-md hover:bg-white/10 cursor-pointer group transition-colors pointer-events-auto ${getIconClasses()}`}
      style={{ left: 0, top: 0 }}
      onDoubleClick={onDoubleClick}
      onContextMenu={onContextMenu}
    >
      {children}
    </motion.div>
  );
};

// --- COMPONENTE DE JANELA (DRAGGABLE, RESIZABLE E ANIMADA) ---
const DesktopWindow = ({ app, onClose, onMinimize, onMaximize, onFocus, isFullscreen, onUpdateBounds, displays, onSaveBounds }: any) => {
  const dragControls = useDragControls();
  const [isResizing, setIsResizing] = useState(false);
  const [isDragging, setIsDragging] = useState(false);
  const windowRef = useRef<HTMLDivElement>(null);

  // Local bounds for high-performance drag and resize (prevents global re-renders)
  const [localBounds, setLocalBounds] = useState({ x: app.x, y: app.y, width: app.width || 800, height: app.height || 500 });

  useEffect(() => {
    if (!isDragging && !isResizing) {
      setLocalBounds({ x: app.x, y: app.y, width: app.width || 800, height: app.height || 500 });
    }
  }, [app.x, app.y, app.width, app.height, isDragging, isResizing]);

  // Calcula em qual monitor a janela está baseada no seu X e Y
  const monitor = displays?.find((d: any) => 
    localBounds.x + 50 >= d.logicalX && 
    localBounds.x + 50 <= d.logicalX + d.logicalWidth && 
    localBounds.y + 10 >= d.logicalY && 
    localBounds.y + 10 <= d.logicalY + d.logicalHeight
  ) || displays?.find((d: any) => d.isPrimary) || displays?.[0];

  // Handles de resize manuais
  const handleResize = (e: any, direction: string) => {
    if (app.maximized || isFullscreen) return;
    
    e.stopPropagation();
    setIsResizing(true);
    const startX = e.clientX;
    const startY = e.clientY;
    const startW = localBounds.width;
    const startH = localBounds.height;
    const startPosX = localBounds.x;
    const startPosY = localBounds.y;

    const onMouseMove = (moveEvent: any) => {
      let newW = startW;
      let newH = startH;
      let newX = startPosX;
      let newY = startPosY;

      const deltaX = moveEvent.clientX - startX;
      const deltaY = moveEvent.clientY - startY;

      if (direction.includes('right')) newW = Math.max(300, startW + deltaX);
      if (direction.includes('bottom')) newH = Math.max(200, startH + deltaY);
      if (direction.includes('left')) {
        const potentialW = startW - deltaX;
        if (potentialW >= 300) {
          newW = potentialW;
          newX = startPosX + deltaX;
        }
      }
      if (direction.includes('top')) {
        const potentialH = startH - deltaY;
        if (potentialH >= 200) {
          newH = potentialH;
          newY = startPosY + deltaY;
        }
      }

      setLocalBounds({ width: newW, height: newH, x: newX, y: newY });
    };

    const onMouseUp = () => {
      setIsResizing(false);
      document.removeEventListener('mousemove', onMouseMove);
      document.removeEventListener('mouseup', onMouseUp);
      
      setLocalBounds(current => {
         onUpdateBounds(app.id, { width: current.width, height: current.height, x: current.x, y: current.y });
         if (onSaveBounds) onSaveBounds(app.id);
         return current;
      });
    };

    document.addEventListener('mousemove', onMouseMove);
    document.addEventListener('mouseup', onMouseUp);
  };

  return (
    <motion.div
      ref={windowRef}
      initial={{ opacity: 0, scale: 0.8, y: 50 }}
      animate={
        app.minimized ? {
          opacity: 0,
          scale: 0.4,
          y: monitor ? monitor.logicalY + monitor.physicalHeight : window.innerHeight,
          x: localBounds.x,
          width: app.maximized || isFullscreen ? (monitor ? monitor.physicalWidth : '100vw') : localBounds.width,
          height: app.maximized || isFullscreen ? (monitor ? monitor.physicalHeight : '100vh') : localBounds.height
        } : { 
          opacity: 1, 
          scale: 1, 
          x: app.maximized || isFullscreen ? (monitor ? monitor.logicalX : 0) : localBounds.x, 
          y: app.maximized || isFullscreen ? (monitor ? monitor.logicalY : 0) : localBounds.y,
          width: app.maximized || isFullscreen ? (monitor ? monitor.logicalWidth : '100vw') : localBounds.width,
          height: isFullscreen 
                  ? (monitor ? monitor.logicalHeight : '100vh') 
                  : app.maximized 
                    ? (monitor ? (monitor.logicalHeight - 80) : '100vh')
                    : localBounds.height
        }
      }
      exit={{ opacity: 0, scale: 0.8, y: 50 }}
      transition={
        isResizing || isDragging ? { duration: 0 } : { 
          type: "spring", 
          stiffness: 300, 
          damping: 30,
          mass: 1.5
        }
      }
      onClick={onFocus}
      drag={!isFullscreen}
      dragControls={dragControls}
      dragListener={false}
      dragMomentum={false}
      style={{ 
        zIndex: isFullscreen ? 99999 : app.zIndex,
        position: 'absolute',
        top: 0, left: 0,
        pointerEvents: app.minimized ? 'none' : 'auto',
        willChange: 'transform, opacity, width, height'
      }}
      onDragStart={() => {
        setIsDragging(true);
        if (app.maximized) {
          const rect = windowRef.current?.getBoundingClientRect();
          const w = rect?.width || window.innerWidth;
          const h = rect?.height || window.innerHeight;
          setLocalBounds(prev => ({ ...prev, width: w, height: h, x: 0, y: 0 }));
          onUpdateBounds(app.id, { maximized: false, width: w, height: h, x: 0, y: 0 });
        }
      }}
      onDragEnd={() => {
        setIsDragging(false);
        
        setLocalBounds(current => {
          let finalX = current.x;
          let finalY = current.y;

          if (displays && displays.length > 0) {
            const titleX = finalX + 50;
            const titleY = finalY + 10;
            
            const isInsideAnyMonitor = displays.some((d: any) => 
              titleX >= d.logicalX && 
              titleX <= d.logicalX + d.logicalWidth && 
              titleY >= d.logicalY && 
              titleY <= d.logicalY + d.logicalHeight
            );

            if (!isInsideAnyMonitor) {
              const primary = displays.find((d: any) => d.isPrimary) || displays[0];
              finalX = primary.logicalX + 100;
              finalY = primary.logicalY + 100;
            }
          }
          
          onUpdateBounds(app.id, { x: finalX, y: finalY });
          if (onSaveBounds) onSaveBounds(app.id);
          return { ...current, x: finalX, y: finalY };
        });
      }}
      onDrag={(_, info) => {
        setLocalBounds(prev => ({ ...prev, x: prev.x + info.delta.x, y: prev.y + info.delta.y }));
      }}
      className={`absolute flex flex-col shadow-2xl ${
        isFullscreen ? 'rounded-none border-none z-[99999]' : 
        app.maximized ? 'rounded-none border-none' : 
        'rounded-xl border border-white/20 glass'
      }`}
    >
      {/* Custom Resize Handles (Só aparecem se não tiver maximizado) */}
      {!app.maximized && !isFullscreen && (
        <>
          <div className="resize-handle resize-top" onMouseDown={(e) => handleResize(e, 'top')} />
          <div className="resize-handle resize-bottom" onMouseDown={(e) => handleResize(e, 'bottom')} />
          <div className="resize-handle resize-left" onMouseDown={(e) => handleResize(e, 'left')} />
          <div className="resize-handle resize-right" onMouseDown={(e) => handleResize(e, 'right')} />
          <div className="resize-handle resize-top-left" onMouseDown={(e) => handleResize(e, 'top-left')} />
          <div className="resize-handle resize-top-right" onMouseDown={(e) => handleResize(e, 'top-right')} />
          <div className="resize-handle resize-bottom-left" onMouseDown={(e) => handleResize(e, 'bottom-left')} />
          <div className="resize-handle resize-bottom-right" onMouseDown={(e) => handleResize(e, 'bottom-right')} />
        </>
      )}

      {/* Title bar (Hidden in true fullscreen) */}
      {!isFullscreen && (
        <div 
          onPointerDown={(e) => dragControls.start(e)}
          onDoubleClick={onMaximize}
          className={`bg-black/80 p-3 flex justify-between items-center cursor-default border-b border-white/10 select-none shrink-0 ${app.maximized ? '' : 'rounded-t-xl'}`}
        >
          <div className="flex gap-2 pl-2">
             <div className="w-3.5 h-3.5 rounded-full bg-[#ff5f56] cursor-pointer hover:bg-red-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); onClose(); }}><X size={10} className="opacity-0 group-hover:opacity-100 text-black"/></div>
             <div className="w-3.5 h-3.5 rounded-full bg-[#ffbd2e] cursor-pointer hover:bg-yellow-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); onMinimize(); }}><span className="opacity-0 group-hover:opacity-100 text-black text-[10px] font-bold">-</span></div>
             <div className="w-3.5 h-3.5 rounded-full bg-[#27c93f] cursor-pointer hover:bg-green-400 transition-colors flex items-center justify-center group" onClick={(e) => { e.stopPropagation(); onMaximize(); }}><span className="opacity-0 group-hover:opacity-100 text-black text-[10px] font-bold">+</span></div>
          </div>
          <span className="text-xs font-semibold text-white/80">{app.title}</span>
          <div className="w-12"></div>
        </div>
      )}
      {/* Content */}
      <div className={`flex-1 bg-[#1e1e1e] relative overflow-hidden pointer-events-auto ${isFullscreen || app.maximized ? '' : 'rounded-b-xl'}`}>
        {app.content}
      </div>
    </motion.div>
  );
};


function App() {
  const { theme, wallpapers, isLoading } = useTheme();
  const { displays } = useDisplay();
  const [desktopShortcuts, setDesktopShortcuts] = useState<any[]>([]);
  const [desktopIconSize, setDesktopIconSize] = useState<'small' | 'medium' | 'large'>('medium');
  const [showDesktopIcons, setShowDesktopIcons] = useState<boolean>(true);
  const [desktopContextMenu, setDesktopContextMenu] = useState<{x: number, y: number} | null>(null);
  const [iconContextMenu, setIconContextMenu] = useState<{x: number, y: number, id: string, isCustom: boolean, shortcut?: any} | null>(null);

  // Pega as dimensões totais da janela que agora cobre todos os monitores
  useEffect(() => {
    const handleResize = () => {};
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, []);

  // Estado do Painel de Controle e Menu Iniciar
  const [showControlCenter, setShowControlCenter] = useState(false);
  const [showStartMenu, setShowStartMenu] = useState(false);
  const [showStartContextMenu, setShowStartContextMenu] = useState(false);
  const [isFullscreenMode, setIsFullscreenMode] = useState(false);
  const [activeMonitorId, setActiveMonitorId] = useState<string | null>(null);
  const [iconPositions, setIconPositions] = useState<Record<string, { col: number, row: number }>>({});

  // Gerenciamento Avançado de Janelas
  const [apps, setApps] = useState([
    {
      id: 'browser', baseId: 'browser', title: 'Genesi Browser', icon: Globe, color: 'bg-blue-500', 
      defaultX: 100, defaultY: 50, x: 100, y: 50, width: 800, height: 500,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: (
        <div className="w-full h-full flex flex-col bg-gray-100">
          <div className="bg-white flex items-center p-2 gap-2 text-black border-b border-gray-300">
             <Globe size={16} className="text-gray-500"/>
             <input type="text" value="https://react.dev" readOnly className="bg-gray-100 px-3 py-1.5 rounded-md w-full text-sm outline-none" />
          </div>
          <iframe src="https://react.dev" className="w-full h-full border-none bg-white"></iframe>
        </div>
      )
    },
    {
      id: 'terminal', baseId: 'terminal', title: 'Terminal - root@genesi', icon: Terminal, color: 'bg-gray-800', 
      defaultX: 150, defaultY: 100, x: 150, y: 100, width: 700, height: 450,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: (
        <div className="p-4 font-mono text-sm text-green-400 h-full bg-black/90">
          <p>GenesiOS v1.0 (React/Tauri Env)</p>
          <p>Last login: {new Date().toLocaleTimeString()}</p><br/>
          <p className="text-white"><span className="text-blue-400 font-bold">root@genesi</span>:~$ sudo apt update</p>
          <p>[sudo] password for root:</p>
          <p className="animate-pulse">_</p>
        </div>
      )
    },
    {
      id: 'package', baseId: 'package', title: 'Genesi Package Manager', icon: Package, color: 'bg-purple-500', 
      defaultX: 200, defaultY: 150, x: 200, y: 150, width: 600, height: 400,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: (
        <div className="w-full h-full bg-[#1e1e1e] text-white p-8 flex flex-col items-center justify-center">
          <Package size={64} className="text-purple-400 mb-4" />
          <h2 className="text-2xl font-semibold mb-2">Package Manager</h2>
          <p className="text-white/60 mb-6 text-center">Instale pacotes NPM, Rust ou C++ no seu ambiente.</p>
          <button className="bg-purple-600 hover:bg-purple-500 px-6 py-2 rounded-full transition-colors">Procurar Pacotes</button>
        </div>
      )
    },
    {
      id: 'settings', baseId: 'settings', title: 'Configurações', icon: Settings, color: 'bg-blue-600', 
      defaultX: 250, defaultY: 100, x: 250, y: 100, width: 900, height: 600,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: <SettingsApp />
    },
    {
      id: 'files', baseId: 'files', title: 'File Explorer', icon: Folder, color: 'bg-yellow-500', 
      defaultX: 300, defaultY: 120, x: 300, y: 120, width: 850, height: 550,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: <FileExplorer />
    },
    {
      id: 'taskmgr', baseId: 'taskmgr', title: 'Task Manager', icon: Activity, color: 'bg-blue-500', 
      defaultX: 350, defaultY: 150, x: 350, y: 150, width: 800, height: 500,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: null // Injetado abaixo para evitar problemas de escopo circular com apps e closeApp
    },
    {
      id: 'image-viewer', baseId: 'image-viewer', title: 'Image Viewer', icon: Folder, color: 'bg-purple-500', 
      defaultX: 400, defaultY: 150, x: 400, y: 150, width: 800, height: 600,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: null // dynamically injected
    },
    {
      id: 'video-player', baseId: 'video-player', title: 'Video Player', icon: Play, color: 'bg-red-500', 
      defaultX: 450, defaultY: 180, x: 450, y: 180, width: 800, height: 600,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: null
    },
    {
      id: 'text-editor', baseId: 'text-editor', title: 'Text Editor', icon: List, color: 'bg-gray-500', 
      defaultX: 500, defaultY: 200, x: 500, y: 200, width: 800, height: 600,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: null
    }
  ]);

  const [pinnedApps] = useState(['browser', 'files', 'settings', 'taskmgr', 'chrome']);
  
  const APP_DEF: Record<string, any> = {
    'browser': { title: 'Genesi Browser', icon: Globe, isExternal: true },
    'terminal': { title: 'Terminal', icon: Terminal },
    'package': { title: 'Package Manager', icon: Package },
    'settings': { title: 'Configurações', icon: Settings },
    'files': { title: 'File Explorer', icon: Folder },
    'taskmgr': { title: 'Task Manager', icon: Activity },
    'image-viewer': { title: 'Image Viewer', icon: Folder, isHidden: true },
    'video-player': { title: 'Video Player', icon: Play, isHidden: true },
    'text-editor': { title: 'Text Editor', icon: List, isHidden: true },
    'chrome': { title: 'Google Chrome', icon: IconBrandChrome, isExternal: true },
  };

  // Load saved bounds on mount
  useEffect(() => {
    const loadBounds = async () => {
      try {
        const savedBounds: any = await appStateStore.get('windowBounds');
        if (savedBounds) {
          setApps(prevApps => prevApps.map(app => {
            if (savedBounds[app.id]) {
              return { ...app, ...savedBounds[app.id] };
            }
            return app;
          }));
        }
      } catch (e) {
        console.error('Failed to load window bounds:', e);
      }
    };
    
    const loadShortcuts = async () => {
      try {
        const store = new LazyStore('settings.json');
        const shortcuts = await store.get<any[]>('desktop_shortcuts');
        if (shortcuts && Array.isArray(shortcuts)) {
          setDesktopShortcuts(shortcuts);
        }
        
        const size = await store.get<'small' | 'medium' | 'large'>('desktop_icon_size');
        if (size) setDesktopIconSize(size);

        const show = await store.get<boolean>('show_desktop_icons');
        if (show !== undefined && show !== null) setShowDesktopIcons(show);

        const pos = await store.get<Record<string, { col: number, row: number }>>('desktop_icon_positions');
        if (pos) setIconPositions(pos);

      } catch (e) {
        console.error('Failed to load desktop shortcuts:', e);
      }
    };

    loadBounds();
    loadShortcuts();
    
    // Poll for shortcut changes periodically since FileExplorer might change them
    const interval = setInterval(loadShortcuts, 2000);
    return () => clearInterval(interval);
  }, []);

  const updateIconPosition = async (id: string, dropX: number, dropY: number, defaultIndex: number) => {
    const step = getGridSnapSize();
    
    // Cálculo puramente matemático da célula baseada nas coordenadas de soltura
    const relX = dropX - primaryDisplay.logicalX;
    const relY = dropY - primaryDisplay.logicalY;

    const maxCols = Math.max(1, Math.floor(primaryDisplay.logicalWidth / step));
    const maxRows = Math.max(1, Math.floor((primaryDisplay.logicalHeight - 60) / step));

    let targetCol = Math.floor((relX - 16) / step);
    let targetRow = Math.floor((relY - 16) / step);

    // Limitar para não sair da tela matematicamente
    targetCol = Math.max(0, Math.min(targetCol, maxCols - 1));
    targetRow = Math.max(0, Math.min(targetRow, maxRows - 1));

    setIconPositions(prev => {
      const getDefPos = (idx: number) => ({ col: Math.floor(idx / maxRows), row: idx % maxRows });
      const currentPos = prev[id] || getDefPos(defaultIndex);

      let overlappingId: string | null = null;
      
      const allIds = [
        { id: 'default-trash', idx: 0 },
        { id: 'default-files', idx: 1 },
        { id: 'default-settings', idx: 2 },
        { id: 'default-taskmgr', idx: 3 },
        { id: 'default-chrome', idx: 4 },
        ...desktopShortcuts.map((s, i) => ({ id: `custom-${s.path}`, idx: 5 + i }))
      ];

      for (const item of allIds) {
        if (item.id === id) continue;
        const itemPos = prev[item.id] || getDefPos(item.idx);
        if (itemPos.col === targetCol && itemPos.row === targetRow) {
          overlappingId = item.id;
          break;
        }
      }

      const next = { ...prev, [id]: { col: targetCol, row: targetRow } };
      
      // Se houver sobreposição, faz o swap (troca) matemático para o lugar antigo
      if (overlappingId) {
        next[overlappingId] = currentPos;
      }

      const store = new LazyStore('settings.json');
      store.set('desktop_icon_positions', next).then(() => store.save());
      return next;
    });
  };

  const saveAppBounds = async (id: string) => {
    // Only save when user drops/resizes to prevent heavy disk writes
    setApps(currentApps => {
      const app = currentApps.find(a => a.id === id);
      if (app) {
        appStateStore.get('windowBounds').then((savedBounds: any) => {
          const newBounds = { ...(savedBounds || {}) };
          newBounds[id] = {
            x: app.x,
            y: app.y,
            width: app.width,
            height: app.height,
            maximized: app.maximized
          };
          appStateStore.set('windowBounds', newBounds).then(() => appStateStore.save());
        });
      }
      return currentApps;
    });
  };

  // Funções do Window Manager
  const updateAppBounds = (id: string, boundsOrUpdater: any) => {
    setApps(apps => apps.map(a => {
      if (a.id === id) {
        const newBounds = typeof boundsOrUpdater === 'function' ? boundsOrUpdater(a) : boundsOrUpdater;
        return { ...a, ...newBounds };
      }
      return a;
    }));
  };
  const openApp = async (baseId: string, forceNewInstance = false, additionalProps = {}) => {
    if (baseId === 'chrome') {
      try {
        const { invoke } = await import('@tauri-apps/api/core');
        await invoke('launch_browser_wayland');
      } catch (e) {
        console.error('Failed to start Browser natively in Wayland:', e);
      }
      setShowControlCenter(false);
      setShowStartMenu(false);
      setShowStartContextMenu(false);
      return;
    }

    setApps(currentApps => {
      const existingInstances = currentApps.filter(a => a.baseId === baseId && a.isOpen);
      
      // If it's already open and we don't force a new instance, just focus the first one
      if (!forceNewInstance && existingInstances.length > 0 && Object.keys(additionalProps).length === 0) {
        return currentApps.map(a => 
          a.id === existingInstances[0].id ? { ...a, minimized: false, zIndex: ++globalZIndex } : a
        );
      }

      // Need to open or spawn a new instance
      const defaultInstance = currentApps.find(a => a.id === baseId);
      if (!defaultInstance) return currentApps; // fallback

      // If default is closed and we don't force new, just open the default one
      const targetInstance = (!defaultInstance.isOpen && !forceNewInstance && Object.keys(additionalProps).length === 0)
        ? defaultInstance
        : {
            ...defaultInstance,
            id: `${baseId}_${Date.now()}`,
            x: defaultInstance.defaultX + (existingInstances.length * 30),
            y: defaultInstance.defaultY + (existingInstances.length * 30),
            ...additionalProps
          };

      // Abre como janela nativa do Tauri se for um app interno do GenesiOS
      if (!targetInstance.isExternal && baseId !== 'terminal') {
        try {
          const webview = new WebviewWindow(targetInstance.id, {
            url: `index.html?app=${baseId}&path=${encodeURIComponent((additionalProps as any).filePath || (additionalProps as any).defaultPath || '')}&name=${encodeURIComponent((additionalProps as any).fileName || '')}`,
            title: targetInstance.title,
            width: targetInstance.width,
            height: targetInstance.height,
            decorations: false, // Nós vamos desenhar a barra de título no React (se for customizada) ou deixar o Wayland por a preta!
            transparent: true,
            center: true
          });

          webview.once('tauri://error', function (e) {
            console.error('Failed to create webview window:', e);
          });
          
          // Quando a janela nativa fechar, atualiza a barra de tarefas do Desktop
          webview.onCloseRequested(() => {
            setApps(apps => apps.map(a => a.id === targetInstance.id ? { ...a, isOpen: false } : a));
          });
        } catch (e) {
          console.error('Failed to spawn WebviewWindow', e);
        }
      }

      const updatedInstance = { ...targetInstance, isOpen: true, minimized: false, zIndex: ++globalZIndex };
      
      if (targetInstance.id === defaultInstance.id) {
        return currentApps.map(a => a.id === baseId ? updatedInstance : a);
      }
      return [...currentApps, updatedInstance];
    });

    setShowControlCenter(false);
    setShowStartMenu(false);
    setShowStartContextMenu(false);
  };

  const closeApp = async (id: string) => {
    setApps(apps => apps.map(a => {
      if (a.id === id) {
        setTimeout(() => saveAppBounds(id), 100);
        return { ...a, isOpen: false, minimized: false };
      }
      return a;
    }));

    // Fecha a janela nativa do Tauri (se existir)
    try {
      const w = WebviewWindow.getByLabel(id);
      if (w) await w.close();
    } catch (e) {
      console.error('Failed to close webview', e);
    }

    // Se for uma instância dinâmica clonada, remova ela do estado após a animação de fechar (300ms)
    setTimeout(() => {
      setApps(currentApps => currentApps.filter(a => !(a.id === id && a.id !== a.baseId && !a.isOpen)));
    }, 300);
  };

  const toggleMinimize = async (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) return { ...a, minimized: !a.minimized, zIndex: a.minimized ? ++globalZIndex : a.zIndex };
      return a;
    }));
    try {
      const w = WebviewWindow.getByLabel(id);
      if (w) {
        const isMin = await w.isMinimized();
        if (isMin) await w.unminimize();
        else await w.minimize();
      }
    } catch (e) {
      console.error('Failed to toggle minimize webview', e);
    }
  };

  const toggleMaximize = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) {
        const newMaximized = !a.maximized;
        // Schedule save after state update
        setTimeout(() => saveAppBounds(id), 100);
        return { ...a, maximized: newMaximized, zIndex: ++globalZIndex };
      }
      return a;
    }));
  };

  const focusApp = async (id: string) => {
    setApps(apps.map(a => a.id === id ? { ...a, zIndex: ++globalZIndex } : a));
    try {
      const w = WebviewWindow.getByLabel(id);
      if (w) await w.setFocus();
    } catch (e) {
      console.error('Failed to focus webview', e);
    }
  };

  // Injeta o conteúdo dinâmico que depende do state "apps" no Task Manager
  const appsWithDynamicContent = apps.map(a => {
    if (a.baseId === 'browser') {
      return { ...a, content: <BrowserApp /> };
    }
    if (a.id === 'taskmgr') {
      return { ...a, content: <TaskManager apps={apps} onCloseApp={closeApp} /> };
    }
    if (a.baseId === 'files') {
      return { ...a, content: <FileExplorer onOpenInApp={(baseId, props) => openApp(baseId, true, props)} initialPath={(a as any).defaultPath} /> };
    }
    if (a.baseId === 'image-viewer' && (a as any).filePath) {
      return { ...a, content: <ImageViewer filePath={(a as any).filePath} fileName={(a as any).fileName} /> };
    }
    if (a.baseId === 'video-player' && (a as any).filePath) {
      return { ...a, content: <VideoPlayer filePath={(a as any).filePath} fileName={(a as any).fileName} /> };
    }
    if (a.baseId === 'text-editor' && (a as any).filePath) {
      return { ...a, content: <TextEditor filePath={(a as any).filePath} fileName={(a as any).fileName} /> };
    }
    if (a.baseId === 'terminal') {
      return { ...a, content: <TerminalApp /> };
    }
    return a;
  });

  // Lidar com F11 para tela cheia de verdade e Tecla Windows
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'F11') {
        e.preventDefault(); // Impede o browser de roubar o F11 se tiver rodando em web
        setIsFullscreenMode(prev => !prev);
      }
      if (e.key === 'Meta') {
        e.preventDefault();
        setShowStartMenu(prev => !prev);
        setShowControlCenter(false);
        setShowStartContextMenu(false);
      }
      // Atalho para o Task Manager (Ctrl + Shift + Esc)
      if (e.ctrlKey && e.shiftKey && e.key === 'Escape') {
        e.preventDefault();
        openApp('taskmgr');
      }
      
      // ALT + TAB Switcher Logic
      if (e.altKey && e.key === 'Tab') {
        e.preventDefault();
        setApps(currentApps => {
           const openAppsList = currentApps.filter(a => a.isOpen);
           if (openAppsList.length <= 1) return currentApps;
           
           // Sort by zIndex descending to find the top two windows
           const sorted = [...openAppsList].sort((a, b) => b.zIndex - a.zIndex);
           
           // The currently focused app is sorted[0]. The next app to focus is sorted[1].
           const nextApp = sorted[1];
           
           // Focus the next app
           return currentApps.map(a => a.id === nextApp.id ? { ...a, zIndex: ++globalZIndex } : a);
        });
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  const handleDesktopContextMenu = (e: React.MouseEvent) => {
    if (e.target === e.currentTarget) {
      e.preventDefault();
      setIconContextMenu(null);
      setDesktopContextMenu({ x: e.clientX, y: e.clientY });
      setShowStartMenu(false);
      setShowControlCenter(false);
    }
  };

  const handleUpdateDesktopIcons = async (updates: { size?: 'small' | 'medium' | 'large', show?: boolean }) => {
    const store = new LazyStore('settings.json');
    if (updates.size) {
      setDesktopIconSize(updates.size);
      await store.set('desktop_icon_size', updates.size);
    }
    if (updates.show !== undefined) {
      setShowDesktopIcons(updates.show);
      await store.set('show_desktop_icons', updates.show);
    }
    await store.save();
    setDesktopContextMenu(null);
  };

  const getIconClasses = () => {
    switch(desktopIconSize) {
      case 'small': return 'w-16 h-16 text-[10px] gap-1';
      case 'large': return 'w-24 h-24 text-sm gap-2';
      case 'medium':
      default: return 'w-20 h-20 text-xs gap-1';
    }
  };

  const getIconSize = () => {
    switch(desktopIconSize) {
      case 'small': return 32;
      case 'large': return 48;
      case 'medium':
      default: return 40;
    }
  };

  const getGridSnapSize = () => {
    switch(desktopIconSize) {
      case 'small': return 80;
      case 'large': return 120;
      case 'medium':
      default: return 96;
    }
  };

  // If theme/wallpaper is still loading from the store, don't render the main app yet
  if (isLoading || displays.length === 0) {
    return <div className="w-screen h-screen bg-black flex items-center justify-center text-white font-sans">Loading...</div>;
  }

  // Monitor Principal
  const primaryDisplay = displays.find(d => d.isPrimary) || displays[0];

  return (
    <div className={`fixed inset-0 overflow-hidden flex flex-col items-center justify-center pb-24 ${theme === 'light' ? 'text-black' : 'text-white'}`}
         onClick={() => { setShowControlCenter(false); setShowStartMenu(false); setShowStartContextMenu(false); }}
         onContextMenu={(e) => e.preventDefault()}>

      
      {/* Background/Wallpapers for each monitor based on logical position */}
      {displays.map((d, i) => {
        const bgUrl = wallpapers[d.id] || wallpapers['all'] || "https://images.unsplash.com/photo-1707343843437-caacff5cfa74?q=80&w=3375&auto=format&fit=crop";
        return (
          <div 
            key={`bg-${d.id}`}
            className="absolute z-0"
            style={{
              left: d.logicalX,
              top: d.logicalY,
              width: d.logicalWidth,
              height: d.logicalHeight,
              backgroundImage: `url(${bgUrl})`,
              backgroundSize: 'cover',
              backgroundPosition: 'center',
              backgroundRepeat: 'no-repeat',
              borderRight: displays.length > 1 ? '1px solid rgba(255,255,255,0.05)' : 'none' // Subtile divisor
            }}
            onContextMenu={handleDesktopContextMenu}
            onClick={() => {
              setDesktopContextMenu(null);
              setIconContextMenu(null);
            }}
          />
        );
      })}

      {/* Desktop Context Menu */}
      {desktopContextMenu && (
        <div 
          className={`absolute z-[999999] w-48 py-1 rounded-md shadow-xl border text-[13px] ${
            theme === 'light' ? 'bg-white border-black/10 text-black' : 'bg-[#2d2d2d] border-white/10 text-white'
          }`}
          style={{ top: desktopContextMenu.y, left: desktopContextMenu.x }}
          onClick={(e) => e.stopPropagation()}
          onContextMenu={(e) => e.preventDefault()}
        >
          <div className="px-4 py-1.5 font-semibold opacity-50 cursor-default">View</div>
          <div 
            className={`px-6 py-1.5 cursor-pointer flex items-center justify-between ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={() => handleUpdateDesktopIcons({ size: 'large' })}
          >
            Large icons {desktopIconSize === 'large' && '✓'}
          </div>
          <div 
            className={`px-6 py-1.5 cursor-pointer flex items-center justify-between ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={() => handleUpdateDesktopIcons({ size: 'medium' })}
          >
            Medium icons {desktopIconSize === 'medium' && '✓'}
          </div>
          <div 
            className={`px-6 py-1.5 cursor-pointer flex items-center justify-between ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={() => handleUpdateDesktopIcons({ size: 'small' })}
          >
            Small icons {desktopIconSize === 'small' && '✓'}
          </div>
          <div className={`h-[1px] w-full my-1 ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'}`}></div>
          <div 
            className={`px-4 py-1.5 cursor-pointer flex items-center justify-between ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={() => handleUpdateDesktopIcons({ show: !showDesktopIcons })}
          >
            Show desktop icons {showDesktopIcons && '✓'}
          </div>
          <div className={`h-[1px] w-full my-1 ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'}`}></div>
          <div 
            className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={() => { openApp('settings'); setDesktopContextMenu(null); }}
          >
            Personalize
          </div>
        </div>
      )}

      {/* Icon Context Menu */}
      {iconContextMenu && (
        <div 
          className={`absolute z-[999999] w-48 py-1 rounded-md shadow-xl border text-[13px] ${
            theme === 'light' ? 'bg-white border-black/10 text-black' : 'bg-[#2d2d2d] border-white/10 text-white'
          }`}
          style={{ top: iconContextMenu.y, left: iconContextMenu.x }}
          onClick={(e) => e.stopPropagation()}
          onContextMenu={(e) => e.preventDefault()}
        >
          <div 
            className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            onClick={async () => {
              if (iconContextMenu.id === 'default-trash') {
                try {
                  const cmd = Command.create('open-path', ['/c', 'start', 'shell:RecycleBinFolder']);
                  await cmd.execute();
                } catch (e) {
                  console.error('Failed to open recycle bin', e);
                }
              } else if (iconContextMenu.id === 'default-files') {
                openApp('files');
              } else if (iconContextMenu.id === 'default-settings') {
                openApp('settings');
              } else if (iconContextMenu.id === 'default-taskmgr') {
                openApp('taskmgr');
              } else if (iconContextMenu.id === 'default-chrome') {
                openApp('chrome');
              } else if (iconContextMenu.isCustom && iconContextMenu.shortcut) {
                const shortcut = iconContextMenu.shortcut;
                if (shortcut.is_dir) {
                  openApp('files', true, { defaultPath: shortcut.path });
                } else {
                  try {
                    const { openPath } = await import('@tauri-apps/plugin-opener');
                    await openPath(shortcut.path);
                  } catch (e) {
                    console.error('Failed to open shortcut file natively', e);
                  }
                }
              }
              setIconContextMenu(null);
            }}
          >
            Open
          </div>
          
          {iconContextMenu.id === 'default-trash' && (
            <div 
              className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
              onClick={async () => {
                try {
                  const cmd = Command.create('open-path', ['/c', 'PowerShell.exe', '-NoProfile', '-Command', 'Clear-RecycleBin -Force']);
                  await cmd.execute();
                } catch (e) {
                  console.error('Failed to empty recycle bin', e);
                }
                setIconContextMenu(null);
              }}
            >
              Empty Recycle Bin
            </div>
          )}

          {iconContextMenu.isCustom && (
            <>
              <div className={`h-[1px] w-full my-1 ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'}`}></div>
              <div 
                className={`px-4 py-1.5 cursor-pointer text-red-500 ${theme === 'light' ? 'hover:bg-red-50' : 'hover:bg-red-900/30'}`}
                onClick={async () => {
                  const newShortcuts = desktopShortcuts.filter(s => s.path !== iconContextMenu.shortcut?.path);
                  setDesktopShortcuts(newShortcuts);
                  const store = new LazyStore('settings.json');
                  await store.set('desktop_shortcuts', newShortcuts);
                  await store.save();
                  setIconContextMenu(null);
                }}
              >
                Delete shortcut
              </div>
            </>
          )}
        </div>
      )}

      {/* Desktop Icons - Rendizados no Monitor Principal */}
      {showDesktopIcons && (
        <div 
          className="absolute p-4 pointer-events-none"
          style={{
            left: primaryDisplay.logicalX,
            top: primaryDisplay.logicalY,
            width: primaryDisplay.logicalWidth,
            height: primaryDisplay.logicalHeight - 60 // subtrai a taskbar
          }}
        >
          {/* Recycle Bin (First Icon) */}
          <DesktopIconItem 
            id="default-trash" defaultIndex={0} 
            primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
            iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
            getIconClasses={getIconClasses} onDoubleClick={async () => {
              try {
                const cmd = Command.create('open-path', ['/c', 'start', 'shell:RecycleBinFolder']);
                await cmd.execute();
              } catch (e) {
                console.error('Failed to open recycle bin', e);
              }
            }}
            onContextMenu={(e: React.MouseEvent) => {
              e.preventDefault();
              e.stopPropagation();
              setDesktopContextMenu(null);
              setIconContextMenu({ x: e.clientX, y: e.clientY, id: 'default-trash', isCustom: false });
            }}
          >
            <Trash2 size={getIconSize()} strokeWidth={1} className="text-gray-300 group-hover:scale-105 transition-transform" />
            <span className="font-medium text-white drop-shadow-md truncate w-full text-center px-1">Recycle Bin</span>
          </DesktopIconItem>

          <DesktopIconItem 
            id="default-files" defaultIndex={1} 
            primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
            iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
            getIconClasses={getIconClasses} onDoubleClick={() => openApp('files')}
            onContextMenu={(e: React.MouseEvent) => {
              e.preventDefault();
              e.stopPropagation();
              setDesktopContextMenu(null);
              setIconContextMenu({ x: e.clientX, y: e.clientY, id: 'default-files', isCustom: false });
            }}
          >
            <Folder size={getIconSize()} strokeWidth={1} className="text-yellow-400 group-hover:scale-105 transition-transform" />
            <span className="font-medium text-white drop-shadow-md truncate w-full text-center px-1">Files</span>
          </DesktopIconItem>

          <DesktopIconItem 
            id="default-settings" defaultIndex={2} 
            primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
            iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
            getIconClasses={getIconClasses} onDoubleClick={() => openApp('settings')}
            onContextMenu={(e: React.MouseEvent) => {
              e.preventDefault();
              e.stopPropagation();
              setDesktopContextMenu(null);
              setIconContextMenu({ x: e.clientX, y: e.clientY, id: 'default-settings', isCustom: false });
            }}
          >
            <Settings size={getIconSize()} strokeWidth={1} className="text-gray-300 group-hover:scale-105 transition-transform" />
            <span className="font-medium text-white drop-shadow-md truncate w-full text-center px-1">Settings</span>
          </DesktopIconItem>

          <DesktopIconItem 
            id="default-taskmgr" defaultIndex={3} 
            primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
            iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
            getIconClasses={getIconClasses} onDoubleClick={() => openApp('taskmgr')}
            onContextMenu={(e: React.MouseEvent) => {
              e.preventDefault();
              e.stopPropagation();
              setDesktopContextMenu(null);
              setIconContextMenu({ x: e.clientX, y: e.clientY, id: 'default-taskmgr', isCustom: false });
            }}
          >
            <Activity size={getIconSize()} strokeWidth={1} className="text-blue-400 group-hover:scale-105 transition-transform" />
            <span className="font-medium text-white drop-shadow-md truncate w-full text-center px-1 leading-tight">Task Manager</span>
          </DesktopIconItem>

          <DesktopIconItem 
            id="default-chrome" defaultIndex={4} 
            primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
            iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
            getIconClasses={getIconClasses} onDoubleClick={() => openApp('chrome')}
            onContextMenu={(e: React.MouseEvent) => {
              e.preventDefault();
              e.stopPropagation();
              setDesktopContextMenu(null);
              setIconContextMenu({ x: e.clientX, y: e.clientY, id: 'default-chrome', isCustom: false });
            }}
          >
            <IconBrandChrome size={getIconSize()} strokeWidth={1} className="text-green-500 group-hover:scale-105 transition-transform" />
            <span className="font-medium text-white drop-shadow-md truncate w-full text-center px-1">Chrome</span>
          </DesktopIconItem>

          {/* Custom User Shortcuts */}
          {desktopShortcuts.map((shortcut, i) => {
            const index = i + 5; // offset pelos ícones padrão
            
            return (
            <DesktopIconItem 
              id={`custom-${shortcut.path}`} defaultIndex={index} 
              primaryDisplay={primaryDisplay} getGridSnapSize={getGridSnapSize} 
              iconPositions={iconPositions} updateIconPosition={updateIconPosition} 
              getIconClasses={getIconClasses} key={`sc-${i}`}
              onDoubleClick={async () => {
                if (shortcut.is_dir) {
                  openApp('files', true, { defaultPath: shortcut.path });
                } else {
                  try {
                    const { openPath } = await import('@tauri-apps/plugin-opener');
                    await openPath(shortcut.path);
                  } catch (e) {
                    console.error('Failed to open shortcut file natively', e);
                    // Fallback para abrir no próprio Genesi se for formato suportado
                    const ext = shortcut.name.split('.').pop()?.toLowerCase();
                    if (['jpg', 'jpeg', 'png', 'gif', 'webp'].includes(ext || '')) {
                      openApp('image-viewer', true, { title: `Photos - ${shortcut.name}`, content: null, filePath: shortcut.path, fileName: shortcut.name });
                    } else if (['mp4', 'webm', 'ogg'].includes(ext || '')) {
                      openApp('video-player', true, { title: `Video Player - ${shortcut.name}`, content: null, filePath: shortcut.path, fileName: shortcut.name });
                    } else if (['txt', 'md', 'json', 'js', 'ts', 'jsx', 'tsx', 'css', 'html', 'xml'].includes(ext || '')) {
                      openApp('text-editor', true, { title: `Text Editor - ${shortcut.name}`, content: null, filePath: shortcut.path, fileName: shortcut.name });
                    }
                  }
                }
              }}
              onContextMenu={(e: React.MouseEvent) => {
                e.preventDefault();
                e.stopPropagation();
                setDesktopContextMenu(null);
                setIconContextMenu({ x: e.clientX, y: e.clientY, id: `custom-${shortcut.path}`, isCustom: true, shortcut });
              }}
            >
              {shortcut.is_dir ? (
                 <Folder size={getIconSize()} strokeWidth={1} className="text-yellow-400 group-hover:scale-105 transition-transform" />
              ) : (
                 <div className="w-10 h-10 bg-white/20 rounded flex items-center justify-center group-hover:scale-105 transition-transform">
                   <span className="text-white font-bold">{shortcut.name.split('.').pop()?.toUpperCase() || '?'}</span>
                 </div>
              )}
              <span className="font-medium text-white drop-shadow-md text-center leading-tight truncate w-full px-1" title={shortcut.name}>
                {shortcut.name}
              </span>
              <div className="absolute bottom-6 left-5 w-3 h-3 bg-white rounded-sm flex items-center justify-center">
                <svg width="8" height="8" viewBox="0 0 24 24" fill="none" stroke="black" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round"><path d="M10 9l-6 6 6 6"/><path d="M20 4v7a4 4 0 0 1-4 4H4"/></svg>
              </div>
            </DesktopIconItem>
            );
          })}
        </div>
      )}

      {/* ======= CONTROL CENTER & WIDGETS (Aparece ao clicar no tray) ======= */}
      <AnimatePresence>
        {showControlCenter && (() => {
          const activeMon = displays.find(d => d.id === activeMonitorId) || primaryDisplay;
          const taskbarWidth = Math.min(1100, activeMon.logicalWidth * 0.95);
          // O painel fica à direita, acima da taskbar
          return (
            <ControlCenter 
              show={showControlCenter} 
              x={activeMon.logicalWidth - ((activeMon.logicalWidth / 2) + (taskbarWidth / 2))} 
              y={90}
            />
          );
        })()}
      </AnimatePresence>

      {/* ======= START MENU ======= */}
      {(() => {
        const activeMon = displays.find(d => d.id === activeMonitorId) || primaryDisplay;
        const taskbarWidth = Math.min(1100, activeMon.logicalWidth * 0.95);
        const startMenuX = activeMon.logicalX + (activeMon.logicalWidth / 2) - (taskbarWidth / 2) + 20; // 20px padding left

        return (
          <StartMenu 
            show={showStartMenu} 
            onClose={() => setShowStartMenu(false)} 
            onOpenApp={openApp}
            apps={apps}
            x={startMenuX}
          />
        );
      })()}

      {/* ======= START CONTEXT MENU ======= */}
      <AnimatePresence>
        {showStartContextMenu && (() => {
          const activeMon = displays.find(d => d.id === activeMonitorId) || primaryDisplay;
          const taskbarWidth = Math.min(1100, activeMon.logicalWidth * 0.95);
          const startMenuX = activeMon.logicalX + (activeMon.logicalWidth / 2) - (taskbarWidth / 2) + 20;

          return (
            <StartContextMenu 
              onClose={() => setShowStartContextMenu(false)} 
              onOpenApp={openApp}
              x={startMenuX}
              y={activeMon.logicalY + activeMon.logicalHeight - 80}
            />
          );
        })()}
      </AnimatePresence>

      {/* ======= WINDOW MANAGER ======= */}
      <AnimatePresence>
        {appsWithDynamicContent.filter(a => a.isOpen && (a.baseId === 'terminal')).map(app => (
          <DesktopWindow 
            key={app.id} 
            app={app} 
            isFullscreen={isFullscreenMode && app.zIndex === Math.max(...apps.filter(a => a.isOpen).map(a => a.zIndex))} // Apenas o app no topo ganha o F11
            onUpdateBounds={updateAppBounds}
            onClose={() => closeApp(app.id)}
            onMinimize={() => toggleMinimize(app.id)}
            onMaximize={() => toggleMaximize(app.id)}
            onFocus={() => focusApp(app.id)}
            displays={displays}
            onSaveBounds={saveAppBounds}
          />
        ))}
      </AnimatePresence>

      {/* ======= TASKBARS (UMA POR MONITOR) ======= */}
      {displays.map((d) => (
        <div 
          key={`taskbar-${d.id}`}
          className={`absolute z-[9990] h-[60px] px-5 backdrop-blur-2xl border shadow-2xl rounded-full flex items-center justify-between transition-all duration-300 ${
            theme === 'light' ? 'bg-white/70 border-black/10' : 'bg-black/40 border-white/10'
          }`}
          style={{
            left: d.logicalX + (d.logicalWidth / 2),
            top: d.logicalY + d.logicalHeight - 80, // bottom-5 equivalente
            width: '95%',
            maxWidth: '1100px',
            transform: 'translateX(-50%)'
          }}
          onContextMenu={(e) => e.preventDefault()}
          onClick={(e) => e.stopPropagation()}
        >
          {/* Left - Start Button */}
          <div className="flex items-center gap-4">
            <button 
              onClick={(e) => { 
                e.stopPropagation(); 
                setActiveMonitorId(d.id);
                setShowStartMenu(!showStartMenu); 
                setShowControlCenter(false); 
                setShowStartContextMenu(false); 
              }}
              onContextMenu={(e) => { 
                e.preventDefault(); 
                e.stopPropagation(); 
                setActiveMonitorId(d.id);
                setShowStartContextMenu(true); 
                setShowStartMenu(false); 
              }}
              className={`w-10 h-10 rounded-full flex justify-center items-center transition-colors font-bold text-lg font-mono ${
                theme === 'light' ? 'bg-black/10 hover:bg-black/20 text-black' : 'bg-white/20 hover:bg-white/30 text-white'
              }`}
            >
              G
            </button>
            <div className={`w-[1px] h-6 mx-2 ${theme === 'light' ? 'bg-black/20' : 'bg-white/20'}`}></div>

            {/* Ícones dos apps abertos/pinados na Taskbar */}
            <div className="flex items-center gap-2 relative">
              {pinnedApps.map(baseId => {
                const def = APP_DEF[baseId];
                if (!def) return null;
                const openInstances = apps.filter(a => a.baseId === baseId && a.isOpen);
                const isOpen = openInstances.length > 0;
                const isFocused = openInstances.some(a => a.zIndex === Math.max(...apps.filter(x => x.isOpen).map(x => x.zIndex)));
                
                return (
                  <div 
                    key={`pinned-${baseId}`}
                    className="relative group flex items-center justify-center w-10 h-10"
                  >
                    <button 
                      onClick={(e) => { 
                        e.stopPropagation();
                        if (isOpen && !isFocused) {
                          // Se já está aberto mas não focado, foca ele
                          const instances = apps.filter(a => a.baseId === baseId && a.isOpen);
                          if (instances.length > 0) {
                            focusApp(instances[0].id);
                            if (instances[0].minimized) toggleMinimize(instances[0].id);
                          }
                        } else {
                          // Se não está aberto ou está focado (quer abrir nova janela ou minimizar)
                          openApp(baseId);
                        }
                      }}
                      onContextMenu={(e) => { 
                        e.preventDefault(); 
                        e.stopPropagation();
                        openApp(baseId, true); // Força nova instância no botão direito
                      }} 
                      className={`w-10 h-10 rounded-xl flex justify-center items-center transition-all shadow-lg ${
                        isOpen 
                          ? isFocused 
                            ? (theme === 'light' ? 'bg-black/10 scale-105 border border-black/10' : 'bg-white/20 scale-105 border border-white/10')
                            : (theme === 'light' ? 'bg-black/5 hover:bg-black/10' : 'bg-white/10 hover:bg-white/15')
                          : `bg-transparent border border-transparent ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5'}`
                      }`}
                      title={def.title}
                    >
                      <def.icon size={22} className={isOpen ? (theme === 'light' ? 'text-black' : 'text-white') : (theme === 'light' ? 'text-black/70' : 'text-white/70')} />
                    </button>
                    
                    {/* Bolinha indicadora de aberto */}
                    {isOpen && (
                      <div className={`absolute -bottom-1.5 left-1/2 -translate-x-1/2 h-1 rounded-full transition-all ${
                        isFocused 
                          ? 'w-4 bg-blue-500' 
                          : (theme === 'light' ? 'w-1.5 bg-black/50' : 'w-1.5 bg-white/70')
                      }`}></div>
                    )}

                    {/* Hover Preview (Miniaturas estilo Windows 11) */}
                    {isOpen && (
                      <div className="absolute bottom-[40px] left-1/2 -translate-x-1/2 hidden group-hover:flex transition-opacity pb-4 pt-4 px-10 z-[10000]">
                        <div className={`border rounded-lg shadow-2xl p-2 gap-2 flex pointer-events-auto relative ${
                          theme === 'light' ? 'bg-white/90 border-black/10 backdrop-blur-xl' : 'bg-[#202020] border-white/10'
                        }`}>
                          {openInstances.map(inst => (
                            <div 
                              key={`preview-${inst.id}`} 
                              onClick={(e) => { 
                                e.stopPropagation(); 
                                focusApp(inst.id); 
                                if(inst.minimized) toggleMinimize(inst.id); 
                              }}
                              className={`flex flex-col items-center gap-2 p-2 rounded-md cursor-pointer transition-colors ${
                                theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'
                              }`}
                            >
                              <div className={`w-40 h-24 border rounded overflow-hidden flex items-center justify-center relative group/close shadow-inner ${
                                theme === 'light' ? 'bg-gray-100 border-black/10' : 'bg-black/50 border-white/10'
                              }`}>
                                 {/* Real content preview scaled down */}
                                 {inst.content && !def.isExternal ? (
                                   <div className="absolute inset-0 pointer-events-none origin-top-left" style={{ 
                                     width: inst.width || 800, 
                                     height: inst.height || 500,
                                     transform: `scale(${160 / (inst.width || 800)})` // 160px is the w-40 width
                                   }}>
                                     {/* Re-render the app content inside the thumbnail for live preview */}
                                     {inst.content}
                                     {/* Glass overlay to make it look like a thumbnail */}
                                     <div className={`absolute inset-0 ${theme === 'light' ? 'bg-white/10' : 'bg-black/10'}`}></div>
                                   </div>
                                 ) : (
                                   <def.icon size={32} className={theme === 'light' ? 'text-black/30' : 'text-white/30'} />
                                 )}

                                 {/* Botão fechar instância */}
                                 <div 
                                   className="absolute top-1 right-1 w-6 h-6 flex items-center justify-center bg-red-500/90 hover:bg-red-600 rounded text-white opacity-0 group-hover/close:opacity-100 transition-opacity z-10 shadow-md"
                                   onClick={(e) => { e.stopPropagation(); closeApp(inst.id); }}
                                 >
                                   <X size={14} strokeWidth={2.5} />
                                 </div>
                              </div>
                              <span className={`text-[11px] font-medium truncate w-40 text-center ${
                                theme === 'light' ? 'text-black' : 'text-white'
                              }`}>
                                {inst.title}
                              </span>
                            </div>
                          ))}
                        </div>
                      </div>
                    )}
                  </div>
                );
              })}

              {/* Linha separadora caso tenhamos apps não-fixados abertos no futuro */}
            </div>
          </div>

          {/* TRAY SYSTEM (Right side) */}
          <div className="flex items-center gap-3">
            <div 
              className={`flex items-center gap-4 cursor-pointer p-2 rounded-full transition-colors ml-auto ${
                theme === 'light' ? 'hover:bg-black/5 text-black' : 'hover:bg-white/10 text-white'
              }`}
              onClick={(e) => { e.stopPropagation(); setActiveMonitorId(d.id); setShowControlCenter(!showControlCenter); setShowStartMenu(false); setShowStartContextMenu(false); }}
            >
              <TaskbarClock />
              <div className={`flex items-center gap-2 px-3 py-1.5 rounded-full text-xs shadow-inner ${
                theme === 'light' ? 'bg-black/5 text-black' : 'bg-white/10 text-white'
              }`}>
                <Battery size={14}/>
                <Wifi size={14}/>
                <span className="font-medium">BR</span>
              </div>
            </div>
            
            {/* Show desktop button */}
            <div 
              className={`w-1.5 h-6 border-l ml-1 cursor-pointer transition-colors ${
                theme === 'light' ? 'border-black/20 hover:bg-black/10' : 'border-white/20 hover:bg-white/20'
              }`} 
              onClick={(e) => {
                e.stopPropagation();
                // Minimiza todos os apps abertos
                setApps(prev => prev.map(a => a.isOpen ? { ...a, minimized: true } : a));
              }}
              title="Show Desktop"
            />
          </div>
        </div>
      ))}
    </div>
  );
}

export default App;