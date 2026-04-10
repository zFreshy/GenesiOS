import React, { useState, useEffect, useRef } from 'react';
import { motion, AnimatePresence, useDragControls } from 'framer-motion';
import {
  Wifi, Bluetooth, Bell, Sun, Moon, Battery,
  Search, Globe, Mail, List, Power, Lock, RotateCcw, MoonStar,
  Play, SkipBack, SkipForward, CloudSun, CalendarClock, Settings, X, Terminal, Package
} from 'lucide-react';
import './index.css';

// --- COMPONENTE DE JANELA (DRAGGABLE, RESIZABLE E ANIMADA) ---
const DesktopWindow = ({ app, onClose, onMinimize, onMaximize, onFocus }) => {
  const dragControls = useDragControls();

  if (app.minimized) return null;

  return (
    <motion.div
      initial={{ opacity: 0, scale: 0.8, y: 50 }}
      animate={{ opacity: 1, scale: 1, y: 0 }}
      exit={{ opacity: 0, scale: 0.8, y: 50 }}
      transition={{ type: "spring", stiffness: 300, damping: 25 }}
      onClick={onFocus}
      drag={!app.maximized}
      dragControls={dragControls}
      dragListener={false} // Só deixa arrastar pela titlebar
      dragMomentum={false}
      className={`absolute glass flex flex-col overflow-hidden border border-white/20 shadow-2xl ${app.maximized ? 'inset-0 w-full h-full rounded-none' : 'w-[800px] h-[500px] rounded-3xl'}`}
      style={{ 
        zIndex: app.zIndex, 
        ...(app.maximized ? { left: 0, top: 0 } : { top: app.defaultY, left: app.defaultX }) 
      }}
    >
      {/* Title bar */}
      <div 
        onPointerDown={(e) => dragControls.start(e)}
        onDoubleClick={onMaximize}
        className="bg-black/60 p-3 flex justify-between items-center cursor-move border-b border-white/10 select-none"
      >
        <div className="flex gap-2 pl-2">
           <div className="w-3.5 h-3.5 rounded-full bg-red-500 cursor-pointer hover:bg-red-400 transition-colors" onClick={(e) => { e.stopPropagation(); onClose(); }}></div>
           <div className="w-3.5 h-3.5 rounded-full bg-yellow-500 cursor-pointer hover:bg-yellow-400 transition-colors" onClick={(e) => { e.stopPropagation(); onMinimize(); }}></div>
           <div className="w-3.5 h-3.5 rounded-full bg-green-500 cursor-pointer hover:bg-green-400 transition-colors" onClick={(e) => { e.stopPropagation(); onMaximize(); }}></div>
        </div>
        <span className="text-xs font-semibold text-white/80">{app.title}</span>
        <div className="w-12"></div>
      </div>
      {/* Content */}
      <div className="flex-1 bg-white/5 relative overflow-hidden pointer-events-auto">
        {app.content}
      </div>
    </motion.div>
  );
};


function App() {
  const [time, setTime] = useState(new Date());

  // Estado do Painel de Controle (Oculto por padrão)
  const [showControlCenter, setShowControlCenter] = useState(false);

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
      id: 'browser', title: 'Genesi Browser', icon: Globe, color: 'bg-blue-500', defaultX: 100, defaultY: 50,
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
      id: 'terminal', title: 'Terminal - root@genesi', icon: Terminal, color: 'bg-gray-800', defaultX: 150, defaultY: 100,
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
      id: 'package', title: 'Genesi Package Manager', icon: Package, color: 'bg-purple-500', defaultX: 200, defaultY: 150,
      isOpen: false, minimized: false, maximized: false, zIndex: 10,
      content: (
        <div className="w-full h-full bg-[#1e1e1e] text-white p-8 flex flex-col items-center justify-center">
          <Package size={64} className="text-purple-400 mb-4" />
          <h2 className="text-2xl font-semibold mb-2">Package Manager</h2>
          <p className="text-white/60 mb-6">Instale pacotes NPM, Rust ou C++ no seu ambiente.</p>
          <button className="bg-purple-600 hover:bg-purple-500 px-6 py-2 rounded-full transition-colors">Procurar Pacotes</button>
        </div>
      )
    }
  ]);

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000);
    return () => clearInterval(timer);
  }, []);

  // Funções do Window Manager
  const openApp = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) return { ...a, isOpen: true, minimized: false, zIndex: Date.now() };
      return a;
    }));
    setShowControlCenter(false); // Fecha o control center se abrir um app
  };

  const closeApp = (id: string) => {
    setApps(apps.map(a => a.id === id ? { ...a, isOpen: false, minimized: false, maximized: false } : a));
  };

  const toggleMinimize = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) return { ...a, minimized: !a.minimized, zIndex: a.minimized ? Date.now() : a.zIndex };
      return a;
    }));
  };

  const toggleMaximize = (id: string) => {
    setApps(apps.map(a => {
      if (a.id === id) return { ...a, maximized: !a.maximized, zIndex: Date.now() };
      return a;
    }));
  };

  const focusApp = (id: string) => {
    setApps(apps.map(a => a.id === id ? { ...a, zIndex: Date.now() } : a));
  };

  return (
    <div className="relative w-screen h-screen bg-cover bg-center overflow-hidden flex flex-col items-center justify-center pb-24" 
         style={{ backgroundImage: "url('/wallpaper1.png')" }}
         onClick={() => setShowControlCenter(false)}>
      
      {/* ======= CONTROL CENTER & WIDGETS (Aparece ao clicar no tray) ======= */}
      <AnimatePresence>
        {showControlCenter && (
          <motion.div 
            initial={{ opacity: 0, y: 50, scale: 0.95 }}
            animate={{ opacity: 1, y: 0, scale: 1 }}
            exit={{ opacity: 0, y: 50, scale: 0.95 }}
            transition={{ type: "spring", stiffness: 300, damping: 30 }}
            onClick={(e) => e.stopPropagation()} // Impede que o clique feche o painel
            className="absolute bottom-24 right-5 z-50 origin-bottom-right grid grid-cols-4 auto-rows-[120px] gap-4 w-[850px] p-5 glass bg-black/60 shadow-2xl"
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

      {/* ======= WINDOW MANAGER ======= */}
      <AnimatePresence>
        {apps.filter(a => a.isOpen).map(app => (
          <DesktopWindow 
            key={app.id} 
            app={app} 
            onClose={() => closeApp(app.id)}
            onMinimize={() => toggleMinimize(app.id)}
            onMaximize={() => toggleMaximize(app.id)}
            onFocus={() => focusApp(app.id)}
          />
        ))}
      </AnimatePresence>

      {/* ======= TASKBAR (BOTTOM) ======= */}
      <div 
        onClick={(e) => e.stopPropagation()} 
        className="absolute bottom-5 left-1/2 -translate-x-1/2 h-[60px] px-5 bg-black/40 backdrop-blur-2xl border border-white/10 shadow-2xl rounded-full flex items-center justify-between w-[95%] max-w-[1100px] z-[100]"
      >
         <div className="flex items-center gap-4">
            <button className="w-10 h-10 rounded-full bg-white/20 hover:bg-white/30 flex justify-center items-center transition-colors">
               <Search size={18}/>
            </button>
            
            {/* Ícones dos apps abertos/pinados na Taskbar */}
            {apps.map(app => {
              if (!app.isOpen) return null;
              return (
                <div key={app.id} className="relative group">
                   <button 
                      onClick={() => {
                        if (app.minimized) {
                          toggleMinimize(app.id);
                        } else if (app.zIndex < Date.now() - 1000) {
                          focusApp(app.id); // Traz pra frente se estiver atrás
                        } else {
                          toggleMinimize(app.id); // Minimiza se já estiver na frente
                        }
                      }} 
                      className={`w-10 h-10 rounded-full ${app.color} flex justify-center items-center transition-transform group-hover:-translate-y-1 shadow-lg`}
                    >
                      <app.icon size={18} color="white"/>
                   </button>
                   <div className="absolute -bottom-2 left-1/2 -translate-x-1/2 w-1.5 h-1.5 bg-white rounded-full"></div>
                </div>
              );
            })}
         </div>
         
         <div className="absolute left-1/2 -translate-x-1/2">
            <div className="w-12 h-1.5 bg-white/30 rounded-full"></div>
         </div>
         
         {/* TRAY SYSTEM (Clica aqui para abrir o Painel) */}
         <div 
           className="flex items-center gap-4 cursor-pointer hover:bg-white/10 p-2 rounded-full transition-colors"
           onClick={() => setShowControlCenter(!showControlCenter)}
         >
            <div className="flex flex-col items-end">
              <span className="font-semibold text-sm">{time.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'})}</span>
              <span className="text-white/60 text-[10px] uppercase">{time.toLocaleDateString([], {weekday: 'short', day: 'numeric', month: 'short'})}</span>
            </div>
            <div className="flex items-center gap-2 bg-white/10 px-3 py-1.5 rounded-full text-xs shadow-inner">
               <Battery size={14}/>
               <Wifi size={14}/>
               <span className="font-medium">BR</span>
            </div>
         </div>
      </div>
    </div>
  );
}

export default App;