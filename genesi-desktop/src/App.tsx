import React, { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence, useDragControls } from 'framer-motion';
import { LazyStore } from '@tauri-apps/plugin-store';
import {
  Wifi, Bluetooth, Bell, Sun, Moon, Battery,
  Search, Globe, Mail, List, Power, Lock, RotateCcw, MoonStar,
  Play, SkipBack, SkipForward, CloudSun, CalendarClock, Settings, X, Terminal, Package, Folder, Activity
} from 'lucide-react';
import { IconChevronUp, IconDeviceDesktop } from '@tabler/icons-react';
import './index.css';
import StartMenu from './StartMenu';
import StartContextMenu from './StartContextMenu';
import SettingsApp from './SettingsApp';
import FileExplorer from './FileExplorer';
import TaskManager from './TaskManager';
import { useTheme } from './ThemeContext';
import { useDisplay } from './DisplayContext';

let globalZIndex = 10;
const appStateStore = new LazyStore('appState.json');

// --- COMPONENTE DE JANELA (DRAGGABLE, RESIZABLE E ANIMADA) ---
const DesktopWindow = ({ app, onClose, onMinimize, onMaximize, onFocus, isFullscreen, onUpdateBounds, displays, onSaveBounds }) => {
  const dragControls = useDragControls();
  const [isResizing, setIsResizing] = useState(false);
  const [isDragging, setIsDragging] = useState(false);
  const windowRef = useRef(null);

  // Calcula em qual monitor a janela está baseada no seu X e Y
  const monitor = displays?.find(d => 
    app.x + 50 >= d.logicalX && 
    app.x + 50 <= d.logicalX + d.logicalWidth && 
    app.y + 10 >= d.logicalY && 
    app.y + 10 <= d.logicalY + d.logicalHeight
  ) || displays?.find(d => d.isPrimary) || displays?.[0];

  // Handles de resize manuais
  const handleResize = (e, direction) => {
    if (app.maximized || isFullscreen) return;
    
    e.stopPropagation();
    setIsResizing(true);
    const startX = e.clientX;
    const startY = e.clientY;
    const startW = app.width || 800;
    const startH = app.height || 500;
    const startPosX = app.x;
    const startPosY = app.y;

    const onMouseMove = (moveEvent) => {
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
      onDragStart={(event, info) => {
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
          
          const isInsideAnyMonitor = displays.some(d => 
            titleX >= d.logicalX && 
            titleX <= d.logicalX + d.logicalWidth && 
            titleY >= d.logicalY && 
            titleY <= d.logicalY + d.logicalHeight
          );

          if (!isInsideAnyMonitor) {
            // Se perdeu no limbo, teletransporta de volta pro monitor principal
            const primary = displays.find(d => d.isPrimary) || displays[0];
            onUpdateBounds(app.id, { 
              x: primary.logicalX + 100, 
              y: primary.logicalY + 100 
            });
          }
        }
        
        if (onSaveBounds) onSaveBounds(app.id);
      }}
      onDrag={(event, info) => {
        onUpdateBounds(app.id, (prev) => ({ x: prev.x + info.delta.x, y: prev.y + info.delta.y }));
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
  const [time, setTime] = useState(new Date());

  // Pega as dimensões totais da janela que agora cobre todos os monitores
  const [windowBounds, setWindowBounds] = useState({ width: window.innerWidth, height: window.innerHeight });

  useEffect(() => {
    const handleResize = () => setWindowBounds({ width: window.innerWidth, height: window.innerHeight });
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, []);

  // Estado do Painel de Controle e Menu Iniciar
  const [showControlCenter, setShowControlCenter] = useState(false);
  const [showStartMenu, setShowStartMenu] = useState(false);
  const [showStartContextMenu, setShowStartContextMenu] = useState(false);
  const [isFullscreenMode, setIsFullscreenMode] = useState(false);
  const [activeMonitorId, setActiveMonitorId] = useState<string | null>(null);

  // States dos botões
  const [wifiOn, setWifiOn] = useState(true);
  const [btOn, setBtOn] = useState(true);
  const [dndOn, setDndOn] = useState(false);
  const [darkOn, setDarkOn] = useState(true);
  const [brightness, setBrightness] = useState(80);
  const [volume, setVolume] = useState(60);

  // Gerenciamento Avançado de Janelas
  const [apps, setApps] = useState([
    {
      id: 'browser', title: 'Genesi Browser', icon: Globe, color: 'bg-blue-500', 
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
      id: 'terminal', title: 'Terminal - root@genesi', icon: Terminal, color: 'bg-gray-800', 
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
      id: 'package', title: 'Genesi Package Manager', icon: Package, color: 'bg-purple-500', 
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
      id: 'settings', title: 'Configurações', icon: Settings, color: 'bg-blue-600', 
      defaultX: 250, defaultY: 100, x: 250, y: 100, width: 900, height: 600,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: <SettingsApp />
    },
    {
      id: 'files', title: 'File Explorer', icon: Folder, color: 'bg-yellow-500', 
      defaultX: 300, defaultY: 120, x: 300, y: 120, width: 850, height: 550,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: <FileExplorer />
    },
    {
      id: 'taskmgr', title: 'Task Manager', icon: Activity, color: 'bg-blue-500', 
      defaultX: 350, defaultY: 150, x: 350, y: 150, width: 800, height: 500,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: null // Injetado abaixo para evitar problemas de escopo circular com apps e closeApp
    }
  ]);

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);

  // Load saved bounds on mount
  useEffect(() => {
    const loadBounds = async () => {
      try {
        const savedBounds = await appStateStore.get('windowBounds');
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
    loadBounds();
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
  const openApp = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) {
        return { ...a, isOpen: true, minimized: false, zIndex: ++globalZIndex };
      }
      return a;
    }));
    setShowControlCenter(false); // Fecha o control center se abrir um app
    setShowStartMenu(false); // Fecha o start menu
    setShowStartContextMenu(false); // Fecha o menu de contexto
  };

  const closeApp = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) {
        // Schedule save of its final state before closing
        setTimeout(() => saveAppBounds(id), 100);
        return { ...a, isOpen: false, minimized: false };
      }
      return a;
    }));
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
      </div>

      {/* ======= CONTROL CENTER & WIDGETS (Aparece ao clicar no tray) ======= */}
      <AnimatePresence>
        {showControlCenter && (
          <motion.div 
            initial={{ opacity: 0, y: 50, scale: 0.95 }}
            animate={{ opacity: 1, y: 0, scale: 1 }}
            exit={{ opacity: 0, y: 50, scale: 0.95 }}
            transition={{ type: "spring", stiffness: 300, damping: 30 }}
            onClick={(e) => e.stopPropagation()} // Impede que o clique feche o painel
            className="absolute bottom-24 right-5 z-[9995] origin-bottom-right grid grid-cols-4 auto-rows-[120px] gap-4 w-[850px] p-5 glass bg-black/60 shadow-2xl"
          >
            {/* Recent Apps */}
            <div className="glass !bg-white/5 col-span-2 row-span-2 p-5 flex flex-col justify-between items-center border-none">
              <h4 className="text-xs tracking-widest text-white/60 uppercase self-start w-full text-center mb-4">Acesso Rápido</h4>
              <div className="flex gap-5">
                {apps.map(app => (
                  <div key={app.id} onClick={() => openApp(app.id)} className={`app-icon ${app.color}`}>
                    <app.icon size={28} color="white" />
                  </div>
                ))}
              </div>
              <button className="glass-btn mt-4">Todos os apps</button>
            </div>

            {/* Control Center */}
            <div className="glass !bg-white/5 col-span-2 row-span-1 p-5 flex flex-col justify-center gap-4 border-none">
              <div className="flex justify-around gap-4">
                <button onClick={() => setWifiOn(!wifiOn)} className={`toggle-btn ${wifiOn ? 'active' : ''}`}><Wifi size={20}/></button>
                <button onClick={() => setBtOn(!btOn)} className={`toggle-btn ${btOn ? 'active' : ''}`}><Bluetooth size={20}/></button>
                <button onClick={() => setDndOn(!dndOn)} className={`toggle-btn ${dndOn ? 'active' : ''}`}><Bell size={20}/></button>
                <button onClick={() => setDarkOn(!darkOn)} className={`toggle-btn ${darkOn ? 'active' : ''}`}><Moon size={20}/></button>
              </div>
              <div className="flex flex-col gap-2">
                <div className="h-6 w-full bg-black/30 rounded-full overflow-hidden cursor-pointer" onClick={(e) => setVolume(Math.max(10, volume - 10))}>
                  <div className="h-full bg-white/70 transition-all" style={{width: `${volume}%`}}></div>
                </div>
                <div className="h-6 w-full bg-black/30 rounded-full overflow-hidden cursor-pointer" onClick={(e) => setBrightness(Math.max(10, brightness - 10))}>
                  <div className="h-full bg-[#f39c12] transition-all" style={{width: `${brightness}%`}}></div>
                </div>
              </div>
            </div>

            {/* Media Player */}
            <div className="glass !bg-white/5 col-span-2 row-span-1 p-5 flex items-center justify-between border-none">
              <div className="flex flex-col">
                <h5 className="text-xs font-medium text-green-400 mb-1">Spotify</h5>
                <h3 className="text-xl mb-1">Going Crazy</h3>
                <p className="text-xs text-white/60 mb-3">Flip Capella, Otray...</p>
                <div className="flex items-center gap-4 cursor-pointer text-white">
                  <SkipBack size={20} className="hover:text-white/70"/> 
                  <Play size={28} fill="white" className="hover:scale-110 transition-transform"/> 
                  <SkipForward size={20} className="hover:text-white/70"/>
                </div>
              </div>
              <div className="w-20 h-20 bg-gradient-to-br from-pink-500 to-purple-600 rounded-xl flex justify-center items-center font-bold text-sm shadow-lg">
                CRAZY
              </div>
            </div>

            {/* Weather */}
            <div className="glass !bg-white/5 col-span-2 row-span-1 p-5 flex items-center justify-between border-none">
              <div className="flex flex-col items-start gap-1">
                <CloudSun size={40} color="#FFD700" />
                <h2 className="text-3xl font-light">24°</h2>
              </div>
              <div className="text-right text-sm text-white/80">
                <p className="font-semibold mb-1">Ensolarado</p>
                <p>São Paulo, BR</p>
                <p className="text-xs text-white/50 mt-2">{time.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'})}</p>
              </div>
            </div>

          </motion.div>
        )}
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
      {displays.map((d, i) => (
        <div 
          key={`taskbar-${d.id}`}
          className="absolute z-[9990] h-[60px] px-5 bg-black/40 backdrop-blur-2xl border border-white/10 shadow-2xl rounded-full flex items-center justify-between transition-all duration-300"
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
              className="w-10 h-10 rounded-full bg-white/20 hover:bg-white/30 flex justify-center items-center transition-colors font-bold text-lg font-mono text-white"
            >
              G
            </button>
            <div className="w-[1px] h-6 bg-white/20 mx-2"></div>

            {/* Ícones dos apps abertos/pinados na Taskbar */}
            <div className="flex items-center gap-2">
              {apps.map(app => {
                if (!app.isOpen) return null;
                return (
                  <div key={app.id} className="relative group">
                    <button 
                      onClick={(e) => { 
                        e.stopPropagation(); 
                        if (app.minimized) {
                          toggleMinimize(app.id);
                        } else if (app.zIndex < Math.max(...apps.filter(a => a.isOpen).map(a => a.zIndex))) {
                          focusApp(app.id);
                        } else {
                          toggleMinimize(app.id);
                        }
                      }} 
                      className={`w-10 h-10 rounded-xl ${app.color} flex justify-center items-center transition-transform group-hover:-translate-y-1 shadow-lg`}
                      title={app.title}
                    >
                      <app.icon size={18} color="white"/>
                    </button>
                    {/* Bolinha indicadora de aberto */}
                    <div className="absolute -bottom-2 left-1/2 -translate-x-1/2 w-1.5 h-1.5 bg-white rounded-full"></div>
                  </div>
                );
              })}
            </div>
          </div>

          {/* TRAY SYSTEM (Right side) */}
          <div className="flex items-center gap-3">
            <div 
              className="flex items-center gap-4 cursor-pointer hover:bg-white/10 p-2 rounded-full transition-colors ml-auto"
              onClick={(e) => { e.stopPropagation(); setShowControlCenter(!showControlCenter); setShowStartMenu(false); setShowStartContextMenu(false); }}
            >
              <div className="flex flex-col items-end">
                <span className="font-semibold text-sm leading-tight">{time.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'})}</span>
                <span className="text-white/60 text-[10px] uppercase leading-tight mt-0.5">{time.toLocaleDateString([], {weekday: 'short', day: 'numeric', month: 'short'})}</span>
              </div>
              <div className="flex items-center gap-2 bg-white/10 px-3 py-1.5 rounded-full text-xs shadow-inner">
                <Battery size={14}/>
                <Wifi size={14}/>
                <span className="font-medium">BR</span>
              </div>
            </div>
            
            {/* Show desktop button */}
            <div 
              className="w-1.5 h-6 border-l border-white/20 ml-1 hover:bg-white/20 cursor-pointer transition-colors" 
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