import { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence, useDragControls } from 'framer-motion';
import { LazyStore } from '@tauri-apps/plugin-store';
import {
  Wifi, Battery, Globe, Terminal, Package, Folder, Activity, Settings, X, Play, List
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

// --- COMPONENTE DE JANELA (DRAGGABLE, RESIZABLE E ANIMADA) ---
const DesktopWindow = ({ app, onClose, onMinimize, onMaximize, onFocus, isFullscreen, onUpdateBounds, displays, onSaveBounds }: any) => {
  const dragControls = useDragControls();
  const [isResizing, setIsResizing] = useState(false);
  const [isDragging, setIsDragging] = useState(false);
  const windowRef = useRef<HTMLDivElement>(null);

  // Calcula em qual monitor a janela está baseada no seu X e Y
  const monitor = displays?.find((d: any) => 
    app.x + 50 >= d.logicalX && 
    app.x + 50 <= d.logicalX + d.logicalWidth && 
    app.y + 10 >= d.logicalY && 
    app.y + 10 <= d.logicalY + d.logicalHeight
  ) || displays?.find((d: any) => d.isPrimary) || displays?.[0];

  // Handles de resize manuais
  const handleResize = (e: any, direction: string) => {
    if (app.maximized || isFullscreen) return;
    
    e.stopPropagation();
    setIsResizing(true);
    const startX = e.clientX;
    const startY = e.clientY;
    const startW = app.width || 800;
    const startH = app.height || 500;
    const startPosX = app.x;
    const startPosY = app.y;

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

      onUpdateBounds(app.id, { width: newW, height: newH, x: newX, y: newY });
    };

    const onMouseUp = () => {
      setIsResizing(false);
      document.removeEventListener('mousemove', onMouseMove);
      document.removeEventListener('mouseup', onMouseUp);
      if (onSaveBounds) onSaveBounds(app.id);
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
          y: monitor ? monitor.logicalY + monitor.physicalHeight : window.innerHeight, // Vai para a taskbar DO SEU MONITOR
          x: app.x,
          width: app.maximized || isFullscreen ? (monitor ? monitor.physicalWidth : '100vw') : (app.width || 800),
          height: app.maximized || isFullscreen ? (monitor ? monitor.physicalHeight : '100vh') : (app.height || 500)
        } : { 
          opacity: 1, 
          scale: 1, 
          x: app.maximized || isFullscreen ? (monitor ? monitor.logicalX : 0) : app.x, 
          y: app.maximized || isFullscreen ? (monitor ? monitor.logicalY : 0) : app.y,
          width: app.maximized || isFullscreen ? (monitor ? monitor.logicalWidth : '100vw') : (app.width || 800),
          height: isFullscreen 
                  ? (monitor ? monitor.logicalHeight : '100vh') 
                  : app.maximized 
                    ? (monitor ? (monitor.logicalHeight - 80) : '100vh') // Subtrai o tamanho da taskbar (80px)
                    : (app.height || 500)
        }
      }
      exit={{ opacity: 0, scale: 0.8, y: 50 }}
      transition={
        isResizing || isDragging ? { duration: 0 } : { // Desliga a animação no resize/drag
          type: "spring", 
          stiffness: 300, 
          damping: 30,
          mass: 1.5 // Deixa a animação um pouco mais fluida e menos travada
        }
      }
      onClick={onFocus}
      drag={!isFullscreen}
      dragControls={dragControls}
      dragListener={false} // Só deixa arrastar pela titlebar
      dragMomentum={false}
      style={{ 
        zIndex: isFullscreen ? 99999 : app.zIndex,
        position: 'absolute',
        top: 0, left: 0, // Resetado porque o framer-motion vai controlar via x/y
        pointerEvents: app.minimized ? 'none' : 'auto', // Impede cliques quando minimizada
        willChange: 'transform, opacity, width, height' // Otimização de performance pesada
      }}
      onDragStart={() => {
        setIsDragging(true);
        if (app.maximized) {
          const rect = windowRef.current?.getBoundingClientRect();
          const w = rect?.width || window.innerWidth;
          const h = rect?.height || window.innerHeight;
          onUpdateBounds(app.id, { maximized: false, width: w, height: h, x: 0, y: 0 });
        }
      }}
      onDragEnd={() => {
        setIsDragging(false);
        
        // Impede que a janela se perca no "vazio" entre os monitores ou fora da tela
        if (displays && displays.length > 0) {
          // Checa se o topo da janela (titlebar) está dentro de algum monitor
          const titleX = app.x + 50;
          const titleY = app.y + 10;
          
          const isInsideAnyMonitor = displays.some((d: any) => 
            titleX >= d.logicalX && 
            titleX <= d.logicalX + d.logicalWidth && 
            titleY >= d.logicalY && 
            titleY <= d.logicalY + d.logicalHeight
          );

          if (!isInsideAnyMonitor) {
            // Se perdeu no limbo, teletransporta de volta pro monitor principal
            const primary = displays.find((d: any) => d.isPrimary) || displays[0];
            onUpdateBounds(app.id, { 
              x: primary.logicalX + 100, 
              y: primary.logicalY + 100 
            });
          }
        }
        
        if (onSaveBounds) onSaveBounds(app.id);
      }}
      onDrag={(_, info) => {
        onUpdateBounds(app.id, (prev: any) => ({ x: prev.x + info.delta.x, y: prev.y + info.delta.y }));
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
    'browser': { title: 'Genesi Browser', icon: Globe },
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
        await Command.create('start-chrome').execute();
      } catch (e) {
        console.error('Failed to start Chrome:', e);
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
      if (!defaultInstance.isOpen && !forceNewInstance && Object.keys(additionalProps).length === 0) {
         return currentApps.map(a => a.id === baseId ? { ...a, isOpen: true, minimized: false, zIndex: ++globalZIndex } : a);
      }

      // Force spawn new instance (e.g. Right Click -> New Window or Opening a File)
      const newId = `${baseId}_${Date.now()}`;
      const offset = existingInstances.length * 30;
      const newInstance = {
        ...defaultInstance,
        id: newId,
        isOpen: true,
        minimized: false,
        maximized: false,
        zIndex: ++globalZIndex,
        x: defaultInstance.defaultX + offset,
        y: defaultInstance.defaultY + offset,
        ...additionalProps // Inject custom title, content, etc if passed
      };
      
      return [...currentApps, newInstance];
    });

    setShowControlCenter(false);
    setShowStartMenu(false);
    setShowStartContextMenu(false);
  };

  const closeApp = (id: string) => {
    setApps(apps => apps.map(a => {
      if (a.id === id) {
        setTimeout(() => saveAppBounds(id), 100);
        return { ...a, isOpen: false, minimized: false };
      }
      return a;
    }));

    // Se for uma instância dinâmica clonada, remova ela do estado após a animação de fechar (300ms)
    setTimeout(() => {
      setApps(currentApps => currentApps.filter(a => !(a.id === id && a.id !== a.baseId && !a.isOpen)));
    }, 300);
  };

  const toggleMinimize = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) return { ...a, minimized: !a.minimized, zIndex: a.minimized ? ++globalZIndex : a.zIndex };
      return a;
    }));
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

  const focusApp = (id: string) => {
    setApps(apps.map(a => a.id === id ? { ...a, zIndex: ++globalZIndex } : a));
  };

  // Injeta o conteúdo dinâmico que depende do state "apps" no Task Manager
  const appsWithDynamicContent = apps.map(a => {
    if (a.id === 'taskmgr') {
      return { ...a, content: <TaskManager apps={apps} onCloseApp={closeApp} /> };
    }
    if (a.baseId === 'files') {
      return { ...a, content: <FileExplorer onOpenInApp={(baseId, props) => openApp(baseId, true, props)} initialPath={a.defaultPath} /> };
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
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

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
      {displays.map(d => {
        const bgUrl = wallpapers[d.id] || wallpapers['all'] || '/wallpaper1.png';
        return (
          <div 
            key={d.id}
            className="absolute"
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
            onContextMenu={(e) => e.preventDefault()}
          />
        );
      })}

      {/* Desktop Icons - Rendizados no Monitor Principal */}
      <div 
        className="absolute p-4 flex flex-col gap-4 flex-wrap content-start"
        style={{
          left: primaryDisplay.logicalX,
          top: primaryDisplay.logicalY,
          width: primaryDisplay.logicalWidth,
          height: primaryDisplay.logicalHeight - 60 // subtrai a taskbar
        }}
      >
        <div 
          onDoubleClick={() => openApp('files')}
          className="w-20 h-20 flex flex-col items-center justify-center gap-1 rounded-md hover:bg-white/10 cursor-pointer group transition-colors"
        >
          <Folder size={40} strokeWidth={1} className="text-yellow-400 group-hover:scale-105 transition-transform" />
          <span className="text-xs font-medium text-white drop-shadow-md">Files</span>
        </div>

        <div 
          onDoubleClick={() => openApp('settings')}
          className="w-20 h-20 flex flex-col items-center justify-center gap-1 rounded-md hover:bg-white/10 cursor-pointer group transition-colors"
        >
          <Settings size={40} strokeWidth={1} className="text-gray-300 group-hover:scale-105 transition-transform" />
          <span className="text-xs font-medium text-white drop-shadow-md">Settings</span>
        </div>

        <div 
          onDoubleClick={() => openApp('taskmgr')}
          className="w-20 h-20 flex flex-col items-center justify-center gap-1 rounded-md hover:bg-white/10 cursor-pointer group transition-colors"
        >
          <Activity size={40} strokeWidth={1} className="text-blue-400 group-hover:scale-105 transition-transform" />
          <span className="text-xs font-medium text-white drop-shadow-md text-center leading-tight">Task<br/>Manager</span>
        </div>

        <div 
          onDoubleClick={() => openApp('chrome')}
          className="w-20 h-20 flex flex-col items-center justify-center gap-1 rounded-md hover:bg-white/10 cursor-pointer group transition-colors"
        >
          <IconBrandChrome size={40} strokeWidth={1} className="text-green-500 group-hover:scale-105 transition-transform" />
          <span className="text-xs font-medium text-white drop-shadow-md">Chrome</span>
        </div>

        {/* Custom User Shortcuts */}
        {desktopShortcuts.map((shortcut, i) => (
          <div 
            key={`sc-${i}`}
            onDoubleClick={() => {
              if (shortcut.is_dir) {
                openApp('files', true, { defaultPath: shortcut.path }); // Need to pass defaultPath somehow? Or currentPath
              } else {
                openApp('files', false); // Open files and let it handle, or trigger a command to open path
                Command.create('open-path', [shortcut.path]).execute().catch(e => console.error(e));
              }
            }}
            className="w-20 h-20 flex flex-col items-center justify-center gap-1 rounded-md hover:bg-white/10 cursor-pointer group transition-colors relative"
          >
            {shortcut.is_dir ? (
               <Folder size={40} strokeWidth={1} className="text-yellow-400 group-hover:scale-105 transition-transform" />
            ) : (
               <div className="w-10 h-10 bg-white/20 rounded flex items-center justify-center group-hover:scale-105 transition-transform">
                 <span className="text-white text-xs font-bold">{shortcut.name.split('.').pop()?.toUpperCase() || '?'}</span>
               </div>
            )}
            <span className="text-[11px] font-medium text-white drop-shadow-md text-center leading-tight truncate w-full px-1" title={shortcut.name}>
              {shortcut.name}
            </span>
            <div className="absolute bottom-6 left-5 w-3 h-3 bg-white rounded-sm flex items-center justify-center">
              <svg width="8" height="8" viewBox="0 0 24 24" fill="none" stroke="black" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round"><path d="M10 9l-6 6 6 6"/><path d="M20 4v7a4 4 0 0 1-4 4H4"/></svg>
            </div>
          </div>
        ))}
      </div>

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
        {appsWithDynamicContent.filter(a => a.isOpen).map(app => (
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
                        openApp(baseId);
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