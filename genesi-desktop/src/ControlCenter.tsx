import React, { useState, useEffect } from 'react';
import { motion } from 'framer-motion';
import { 
  Wifi, Bluetooth, Sun,
  Volume2, SunDim, ChevronRight, Lock, Check,
  CloudSun, Moon, Cloud, Bell, SkipBack, Play, SkipForward
} from 'lucide-react';
import { useTheme } from './ThemeContext';
import { invoke, isTauri } from '@tauri-apps/api/core';

interface ControlCenterProps {
  show: boolean;
  x: number;
  y: number;
}

const MOCK_WIFI_NETWORKS = [
  { name: 'Genesi_5G', secure: true, strength: 3 },
  { name: 'GUEST_NETWORK', secure: false, strength: 2 },
  { name: 'Vivo_Fibra_A2', secure: true, strength: 3 },
  { name: 'TP-Link_Extender', secure: true, strength: 1 },
];

const MOCK_BT_DEVICES = [
  { id: '1', name: 'AirPods Pro', is_connected: false },
  { id: '2', name: 'Magic Mouse', is_connected: true },
  { id: '3', name: 'Keychron K2', is_connected: false },
];

const ControlCenter: React.FC<ControlCenterProps> = ({ show, x, y }) => {
  const { theme, setTheme } = useTheme();
  
  const [time, setTime] = useState(new Date());
  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 60000); // update every minute is enough here
    return () => clearInterval(timer);
  }, []);
  
  const [wifiOn, setWifiOn] = useState(true);
  const [btOn, setBtOn] = useState(false);
  const [dndOn, setDndOn] = useState(false);
  
  const [volume, setVolume] = useState(60);
  const [brightness, setBrightness] = useState(80);

  const [expandedView, setExpandedView] = useState<'wifi' | 'bluetooth' | null>(null);
  
  // WiFi State
  const [connectedWifi, setConnectedWifi] = useState<string | null>('Genesi_5G');
  const [connectingTo, setConnectingTo] = useState<string | null>(null);
  const [wifiNetworks, setWifiNetworks] = useState<any[]>(MOCK_WIFI_NETWORKS);
  const [wifiPassword, setWifiPassword] = useState('');

  // Bluetooth State
  const [btDevices, setBtDevices] = useState<any[]>(MOCK_BT_DEVICES);
  const [connectingBt, setConnectingBt] = useState<string | null>(null);

  // Fetch real networks/devices when expanding view
  useEffect(() => {
    if (!isTauri()) return;

    if (expandedView === 'wifi') {
      invoke('get_wifi_networks')
        .then((networks: any) => {
          if (networks && networks.length > 0) {
            setWifiNetworks(networks.map((n: any) => ({
              name: n.ssid,
              secure: n.security !== 'None',
              strength: Math.min(3, Math.max(1, Math.ceil((n.signal_level + 100) / 20)))
            })));
          }
        })
        .catch(e => console.error("Failed to get wifi networks", e));
    } else if (expandedView === 'bluetooth') {
      invoke('get_bluetooth_devices')
        .then((devices: any) => {
          if (devices && devices.length > 0) {
            setBtDevices(devices);
          }
        })
        .catch(e => console.error("Failed to get bluetooth devices", e));
    }
  }, [expandedView]);

  // Region and Time detection
  const [regionName, setRegionName] = useState('Local');
  const [weather, setWeather] = useState({ temp: 24, desc: 'Ensolarado', icon: CloudSun });

  useEffect(() => {
    // Detect user's timezone/region
    try {
      const timeZone = Intl.DateTimeFormat().resolvedOptions().timeZone;
      // Convert "America/Sao_Paulo" to "São Paulo"
      const city = timeZone.split('/')[1]?.replace('_', ' ') || timeZone;
      setRegionName(city);

      // Simulate a weather change based on hour
      const hour = time.getHours();
      if (hour >= 18 || hour < 6) {
        setWeather({ temp: 18, desc: 'Noite Limpa', icon: Moon });
      } else if (hour >= 6 && hour < 10) {
        setWeather({ temp: 20, desc: 'Nublado', icon: Cloud });
      } else {
        setWeather({ temp: 27, desc: 'Ensolarado', icon: CloudSun });
      }
    } catch (e) {
      console.log('Timezone detection failed', e);
    }
  }, [time.getHours()]);

  const handleWifiConnect = async (name: string) => {
    setConnectingTo(name);
    
    if (isTauri()) {
      try {
        await invoke('connect_wifi', { ssid: name, password: wifiPassword || null });
        setConnectedWifi(name);
        setConnectingTo(null);
        setExpandedView(null);
        setWifiOn(true);
        setWifiPassword('');
      } catch (e) {
        console.error('Failed to connect to wifi:', e);
        alert('Falha ao conectar: ' + e);
        setConnectingTo(null);
      }
    } else {
      // Mock fallback
      setTimeout(() => {
        setConnectedWifi(name);
        setConnectingTo(null);
        setExpandedView(null);
        setWifiOn(true);
        setWifiPassword('');
      }, 1500);
    }
  };

  const handleBluetoothConnect = async (id: string, is_connected: boolean) => {
    if (is_connected) {
      // For simplicity, we just toggle it off in UI for mock, or actual disconnect could be implemented
      setBtDevices(prev => prev.map(d => d.id === id ? { ...d, is_connected: false } : d));
      return;
    }

    setConnectingBt(id);

    if (isTauri()) {
      try {
        await invoke('connect_bluetooth', { id });
        setBtDevices(prev => prev.map(d => d.id === id ? { ...d, is_connected: true } : d));
        setConnectingBt(null);
      } catch (e) {
        console.error('Failed to connect to bluetooth:', e);
        alert('Falha ao conectar: ' + e);
        setConnectingBt(null);
      }
    } else {
      setTimeout(() => {
        setBtDevices(prev => prev.map(d => d.id === id ? { ...d, is_connected: true } : d));
        setConnectingBt(null);
      }, 1500);
    }
  };

  if (!show) return null;

  return (
    <motion.div 
      initial={{ opacity: 0, y: 20, scale: 0.95 }}
      animate={{ opacity: 1, y: 0, scale: 1 }}
      exit={{ opacity: 0, y: 20, scale: 0.95 }}
      transition={{ type: "spring", stiffness: 300, damping: 30 }}
      onClick={(e) => e.stopPropagation()}
      className={`absolute z-[9995] origin-bottom-right w-[360px] rounded-2xl shadow-2xl border flex flex-col gap-4 p-4 ${
        theme === 'light' ? 'bg-white/80 border-black/10 text-black backdrop-blur-2xl' : 'bg-[#1c1c1c]/90 border-white/10 text-white backdrop-blur-2xl'
      }`}
      style={{
        right: x,
        bottom: y
      }}
    >
      {/* QUICK SETTINGS GRID */}
      {!expandedView ? (
        <>
          <div className="grid grid-cols-2 gap-3">
            {/* Wi-Fi Card */}
            <div className={`p-3 rounded-xl border flex flex-col gap-3 transition-colors ${
              wifiOn 
                ? (theme === 'light' ? 'bg-blue-500 text-white border-blue-600' : 'bg-blue-600 text-white border-blue-500')
                : (theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/5 border-white/10')
            }`}>
              <div className="flex justify-between items-start">
                <div 
                  className={`w-8 h-8 rounded-full flex items-center justify-center cursor-pointer transition-colors ${wifiOn ? 'bg-white/20 hover:bg-white/30' : (theme === 'light' ? 'bg-white hover:bg-black/5' : 'bg-white/10 hover:bg-white/20')}`}
                  onClick={() => setWifiOn(!wifiOn)}
                >
                  <Wifi size={16} />
                </div>
                <div 
                  className={`w-6 h-6 rounded-full flex items-center justify-center cursor-pointer hover:bg-black/10 transition-colors ${wifiOn ? 'hover:bg-white/20' : ''}`}
                  onClick={() => setExpandedView('wifi')}
                >
                  <ChevronRight size={16} className={wifiOn ? 'text-white' : 'text-gray-400'} />
                </div>
              </div>
              <div>
                <p className="text-sm font-medium leading-tight">Wi-Fi</p>
                <p className={`text-xs opacity-70 leading-tight truncate ${!wifiOn && 'hidden'}`}>{connectedWifi || 'Não conectado'}</p>
              </div>
            </div>

            {/* Bluetooth Card */}
            <div className={`p-3 rounded-xl border flex flex-col gap-3 transition-colors ${
              btOn 
                ? (theme === 'light' ? 'bg-blue-500 text-white border-blue-600' : 'bg-blue-600 text-white border-blue-500')
                : (theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/5 border-white/10')
            }`}>
              <div className="flex justify-between items-start">
                <div 
                  className={`w-8 h-8 rounded-full flex items-center justify-center cursor-pointer transition-colors ${btOn ? 'bg-white/20 hover:bg-white/30' : (theme === 'light' ? 'bg-white hover:bg-black/5' : 'bg-white/10 hover:bg-white/20')}`}
                  onClick={() => setBtOn(!btOn)}
                >
                  <Bluetooth size={16} />
                </div>
                <div 
                  className={`w-6 h-6 rounded-full flex items-center justify-center cursor-pointer hover:bg-black/10 transition-colors ${btOn ? 'hover:bg-white/20' : ''}`}
                  onClick={() => setExpandedView('bluetooth')}
                >
                  <ChevronRight size={16} className={btOn ? 'text-white' : 'text-gray-400'} />
                </div>
              </div>
              <div>
                <p className="text-sm font-medium leading-tight">Bluetooth</p>
                <p className={`text-xs opacity-70 leading-tight truncate ${!btOn && 'hidden'}`}>Desativado</p>
              </div>
            </div>
          </div>

          <div className="grid grid-cols-2 gap-3">
            {/* Theme Card */}
            <div 
              onClick={() => setTheme(theme === 'light' ? 'dark' : 'light')}
              className={`p-3 rounded-xl border flex items-center gap-3 cursor-pointer transition-colors ${
                theme === 'light' ? 'bg-black/5 border-black/10 hover:bg-black/10' : 'bg-white/5 border-white/10 hover:bg-white/10'
              }`}
            >
              {theme === 'light' ? <Sun size={18} /> : <Moon size={18} />}
              <span className="text-sm font-medium">Tema {theme === 'light' ? 'Claro' : 'Escuro'}</span>
            </div>

            {/* DND Card */}
            <div 
              onClick={() => setDndOn(!dndOn)}
              className={`p-3 rounded-xl border flex items-center gap-3 cursor-pointer transition-colors ${
                dndOn 
                  ? (theme === 'light' ? 'bg-purple-500 text-white border-purple-600' : 'bg-purple-600 text-white border-purple-500')
                  : (theme === 'light' ? 'bg-black/5 border-black/10 hover:bg-black/10' : 'bg-white/5 border-white/10 hover:bg-white/10')
              }`}
            >
              <Bell size={18} />
              <span className="text-sm font-medium">Não perturbe</span>
            </div>
          </div>

          {/* SLIDERS */}
          <div className={`p-4 rounded-xl border flex flex-col gap-4 ${theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/5 border-white/10'}`}>
            <div className="flex items-center gap-3">
              <SunDim size={16} className="text-gray-400" />
              <input 
                type="range" 
                min="10" max="100" 
                value={brightness} 
                onChange={(e) => setBrightness(parseInt(e.target.value))}
                className="w-full h-1 bg-gray-400/30 rounded-full appearance-none outline-none [&::-webkit-slider-thumb]:appearance-none [&::-webkit-slider-thumb]:w-4 [&::-webkit-slider-thumb]:h-4 [&::-webkit-slider-thumb]:bg-white [&::-webkit-slider-thumb]:rounded-full [&::-webkit-slider-thumb]:shadow-md cursor-pointer"
              />
            </div>
            <div className="flex items-center gap-3">
              <Volume2 size={16} className="text-gray-400" />
              <input 
                type="range" 
                min="0" max="100" 
                value={volume} 
                onChange={(e) => setVolume(parseInt(e.target.value))}
                className="w-full h-1 bg-gray-400/30 rounded-full appearance-none outline-none [&::-webkit-slider-thumb]:appearance-none [&::-webkit-slider-thumb]:w-4 [&::-webkit-slider-thumb]:h-4 [&::-webkit-slider-thumb]:bg-white [&::-webkit-slider-thumb]:rounded-full [&::-webkit-slider-thumb]:shadow-md cursor-pointer"
              />
            </div>
          </div>

          {/* MEDIA PLAYER */}
          <div className={`p-4 rounded-xl border flex items-center justify-between ${theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/5 border-white/10'}`}>
            <div className="flex flex-col">
              <h5 className="text-[10px] font-bold text-green-500 uppercase tracking-wider mb-1">Spotify</h5>
              <h3 className="text-sm font-medium leading-tight">Blinding Lights</h3>
              <p className="text-xs text-gray-500 leading-tight mb-2">The Weeknd</p>
              <div className="flex items-center gap-3">
                <SkipBack size={16} className="cursor-pointer hover:opacity-70 transition-opacity" /> 
                <Play size={20} fill="currentColor" className="cursor-pointer hover:scale-110 transition-transform" /> 
                <SkipForward size={16} className="cursor-pointer hover:opacity-70 transition-opacity" />
              </div>
            </div>
            <div className="w-16 h-16 bg-gradient-to-br from-red-500 to-yellow-500 rounded-lg flex justify-center items-center font-bold text-xs shadow-md text-white overflow-hidden">
              <div className="w-full h-full bg-black/20 flex items-center justify-center backdrop-blur-sm">
                VIBE
              </div>
            </div>
          </div>

          {/* WEATHER & TIME */}
          <div className={`p-4 rounded-xl border flex items-center justify-between ${theme === 'light' ? 'bg-black/5 border-black/10' : 'bg-white/5 border-white/10'}`}>
            <div className="flex flex-col gap-1">
              <p className="text-sm font-semibold">{time.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'})}</p>
              <p className="text-xs text-gray-500">{regionName}</p>
            </div>
            <div className="flex items-center gap-2">
              <div className="text-right">
                <p className="text-sm font-semibold">{weather.temp}°</p>
                <p className="text-xs text-gray-500">{weather.desc}</p>
              </div>
              <weather.icon size={24} className={weather.icon === CloudSun ? "text-yellow-500" : "text-blue-400"} />
            </div>
          </div>
        </>
      ) : (
        /* EXPANDED WIFI VIEW */
        <div className="flex flex-col h-[400px]">
          <div className="flex items-center gap-3 mb-4">
            <button 
              onClick={() => setExpandedView(null)}
              className={`p-1.5 rounded-md transition-colors ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
            >
              <ChevronRight size={18} className="rotate-180" />
            </button>
            <h3 className="font-medium">{expandedView === 'wifi' ? 'Wi-Fi' : 'Bluetooth'}</h3>
            <div className="ml-auto">
              <div 
                className={`w-10 h-5 rounded-full p-0.5 cursor-pointer transition-colors ${
                  (expandedView === 'wifi' ? wifiOn : btOn) ? 'bg-blue-500' : 'bg-gray-400'
                }`}
                onClick={() => { 
                  if (expandedView === 'wifi') {
                    setWifiOn(!wifiOn); 
                    if(wifiOn) setConnectedWifi(null); 
                  } else {
                    setBtOn(!btOn);
                  }
                }}
              >
                <div className={`w-4 h-4 bg-white rounded-full shadow-sm transition-transform ${
                  (expandedView === 'wifi' ? wifiOn : btOn) ? 'translate-x-5' : 'translate-x-0'
                }`}></div>
              </div>
            </div>
          </div>

          {expandedView === 'wifi' ? (
            wifiOn ? (
              <div className="flex flex-col gap-2 overflow-y-auto pr-2 pb-2">
                {wifiNetworks.map((net, i) => (
                  <div 
                    key={i}
                    className={`p-3 rounded-lg border cursor-pointer transition-all ${
                      connectedWifi === net.name 
                        ? (theme === 'light' ? 'bg-blue-50 border-blue-200' : 'bg-blue-500/20 border-blue-500/30')
                        : (theme === 'light' ? 'bg-white border-black/5 hover:border-black/20' : 'bg-[#252525] border-white/5 hover:border-white/20')
                    }`}
                    onClick={() => {
                      if (connectedWifi !== net.name && !net.secure) {
                        handleWifiConnect(net.name);
                      } else if (connectedWifi !== net.name) {
                        setConnectingTo(net.name);
                      }
                    }}
                  >
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-3">
                        <Wifi size={18} className={connectedWifi === net.name ? 'text-blue-500' : ''} />
                        <div className="flex flex-col">
                          <span className="text-sm font-medium">{net.name}</span>
                          {connectedWifi === net.name ? (
                            <span className="text-xs text-blue-500">Conectado</span>
                          ) : connectingTo === net.name ? (
                            <span className="text-xs text-gray-500">Digite a senha...</span>
                          ) : (
                            <span className="text-xs text-gray-500">{net.secure ? 'Segura' : 'Aberta'}</span>
                          )}
                        </div>
                      </div>
                      {net.secure && connectedWifi !== net.name && <Lock size={14} className="text-gray-400" />}
                      {connectedWifi === net.name && <Check size={16} className="text-blue-500" />}
                    </div>
                    
                    {/* Fake Password Input (Only shows when connecting) */}
                    {connectingTo === net.name && (
                      <div className="mt-3 flex gap-2" onClick={e => e.stopPropagation()}>
                        <input 
                          type="password" 
                          placeholder="Senha da rede" 
                          autoFocus
                          value={wifiPassword}
                          onChange={(e) => setWifiPassword(e.target.value)}
                          onKeyDown={(e) => {
                            if (e.key === 'Enter') handleWifiConnect(net.name);
                          }}
                          className={`flex-1 px-3 py-1.5 text-sm rounded border outline-none ${theme === 'light' ? 'bg-white border-gray-300' : 'bg-black/50 border-gray-600 text-white'}`}
                        />
                        <button 
                          onClick={(e) => { e.stopPropagation(); handleWifiConnect(net.name); }}
                          className="bg-blue-500 text-white px-3 py-1.5 rounded text-sm font-medium"
                        >
                          Conectar
                        </button>
                      </div>
                    )}
                  </div>
                ))}
              </div>
            ) : (
              <div className="flex-1 flex flex-col items-center justify-center text-center px-4 opacity-60">
                <Wifi size={48} className="mb-4 opacity-50" />
                <p className="text-sm">O Wi-Fi está desativado.</p>
                <p className="text-xs mt-1">Ative-o para ver as redes disponíveis e se conectar à internet.</p>
              </div>
            )
          ) : (
            /* BLUETOOTH VIEW */
            btOn ? (
              <div className="flex flex-col gap-2 overflow-y-auto pr-2 pb-2">
                {btDevices.map((dev) => (
                  <div 
                    key={dev.id}
                    className={`p-3 rounded-lg border cursor-pointer transition-all ${
                      dev.is_connected 
                        ? (theme === 'light' ? 'bg-blue-50 border-blue-200' : 'bg-blue-500/20 border-blue-500/30')
                        : (theme === 'light' ? 'bg-white border-black/5 hover:border-black/20' : 'bg-[#252525] border-white/5 hover:border-white/20')
                    }`}
                    onClick={() => handleBluetoothConnect(dev.id, dev.is_connected)}
                  >
                    <div className="flex items-center justify-between">
                      <div className="flex items-center gap-3">
                        <Bluetooth size={18} className={dev.is_connected ? 'text-blue-500' : ''} />
                        <div className="flex flex-col">
                          <span className="text-sm font-medium">{dev.name}</span>
                          {dev.is_connected ? (
                            <span className="text-xs text-blue-500">Conectado</span>
                          ) : connectingBt === dev.id ? (
                            <span className="text-xs text-gray-500">Conectando...</span>
                          ) : (
                            <span className="text-xs text-gray-500">Pareado</span>
                          )}
                        </div>
                      </div>
                      {dev.is_connected && <Check size={16} className="text-blue-500" />}
                    </div>
                  </div>
                ))}
              </div>
            ) : (
              <div className="flex-1 flex flex-col items-center justify-center text-center px-4 opacity-60">
                <Bluetooth size={48} className="mb-4 opacity-50" />
                <p className="text-sm">O Bluetooth está desativado.</p>
                <p className="text-xs mt-1">Ative-o para se conectar a dispositivos.</p>
              </div>
            )
          )}
        </div>
      )}
    </motion.div>
  );
};

export default ControlCenter;
