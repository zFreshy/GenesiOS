import React, { useState, useEffect } from 'react';
import { 
  Activity, BarChart2, FolderOpen, User, Settings,
  PlaySquare, Box, Server, Search, MoreHorizontal, ShieldAlert,
  Cpu, MemoryStick, HardDrive, Wifi,
  ChevronRight, StopCircle, Terminal, Globe
} from 'lucide-react';
import { useTheme } from './ThemeContext';
import { invoke, isTauri } from '@tauri-apps/api/core';

interface TaskManagerProps {
  apps: any[];
  onCloseApp: (id: string) => void;
}

interface ProcessInfo {
  pid: number;
  name: string;
  memory: number;
  cpu: number;
  parent_id: number | null;
}

const TaskManager: React.FC<TaskManagerProps> = ({ apps, onCloseApp }) => {
  const { theme } = useTheme();
  const [activeTab, setActiveTab] = useState<'processes' | 'performance'>('processes');
  const [selectedProcess, setSelectedProcess] = useState<string | null>(null);
  
  // Mock real-time data
  const [cpuUsage, setCpuUsage] = useState(15);
  const [memUsage, setMemUsage] = useState(45);
  
  // Mock dynamic process metrics
  const [processMetrics, setProcessMetrics] = useState<Record<string, { cpu: number, mem: number, disk: number, net: number }>>({});

  const [realProcesses, setRealProcesses] = useState<ProcessInfo[]>([]);
  const [totalMemory, setTotalMemory] = useState<number>(32 * 1024 * 1024 * 1024); // mock fallback
  const [searchQuery, setSearchQuery] = useState('');
  
  const groupedProcesses = React.useMemo(() => {
    if (!isTauri()) return [];

    const groupMap: Record<string, { pids: number[], memory: number, cpu: number, name: string }> = {};

    realProcesses.forEach(p => {
      // Clean up common windows names (e.g., "chrome.exe" -> "Google Chrome", or just group by exe name)
      let baseName = p.name;
      if (baseName.toLowerCase() === 'chrome.exe') baseName = 'Google Chrome';
      else if (baseName.toLowerCase() === 'msedge.exe') baseName = 'Microsoft Edge';
      else if (baseName.toLowerCase() === 'discord.exe') baseName = 'Discord';
      else if (baseName.toLowerCase() === 'code.exe') baseName = 'Visual Studio Code';
      else if (baseName.toLowerCase() === 'explorer.exe') baseName = 'Windows Explorer';

      if (!groupMap[baseName]) {
        groupMap[baseName] = { pids: [], memory: 0, cpu: 0, name: baseName };
      }
      groupMap[baseName].pids.push(p.pid);
      // Para processos com o mesmo nome (geralmente threads/workers que compartilham RSS no Linux),
      // somar a memória gera valores irreais. Pegar o maior valor (Max) é uma heurística melhor para RSS.
      groupMap[baseName].memory = Math.max(groupMap[baseName].memory, p.memory);
      groupMap[baseName].cpu += p.cpu;
    });

    // Convert to array and sort by memory
    const arr = Object.values(groupMap);
    arr.sort((a, b) => b.memory - a.memory);

    // Apply search filter
    if (searchQuery) {
      return arr.filter(g => g.name.toLowerCase().includes(searchQuery.toLowerCase()));
    }
    return arr;
  }, [realProcesses, searchQuery]);

  useEffect(() => {
    // Generate initial metrics for all apps (mock fallback)
    const initialMetrics: any = {
      'system': { cpu: 5.2, mem: 1200, disk: 0.1, net: 0.1 },
      'dwm': { cpu: 1.5, mem: 150, disk: 0, net: 0 },
      'explorer': { cpu: 0.5, mem: 80, disk: 0, net: 0 }
    };
    
    apps.forEach(app => {
      initialMetrics[app.id] = {
        cpu: Math.random() * 2,
        mem: Math.floor(Math.random() * 200) + 50,
        disk: 0,
        net: 0
      };
    });
    
    setProcessMetrics(initialMetrics);

    const updateProcesses = async () => {
      if (isTauri()) {
        try {
          const payload: any = await invoke('get_system_processes');
          setRealProcesses(payload.processes);
          
          // Uso global real via Rust
          setCpuUsage(payload.global_cpu);
          setTotalMemory(payload.total_memory);
          
          // Cálculo real da porcentagem
          const usedMemPct = (payload.used_memory / payload.total_memory) * 100;
          setMemUsage(Math.min(100, Math.max(0, usedMemPct)));
          
          // Distribuir o uso real do GenesiOS (app_process_total) entre os apps abertos do Genesi
          // Vamos adicionar um "Desktop Window Manager (Genesi)" pra absorver uma parte do peso fixo do sistema (ex: 40%)
          // E o restante dividimos entre os apps que o usuario abriu.
          const openApps = apps.filter(a => a.isOpen);
          
          const dwmWeight = 0.4;
          const dwmCpu = payload.genesi_cpu * dwmWeight;
          const dwmMem = (payload.genesi_memory / (1024 * 1024)) * dwmWeight;

          setProcessMetrics(prev => {
             const next = { ...prev };
             
             // Atualiza o DWM
             next['dwm'] = { 
               cpu: dwmCpu * (0.9 + Math.random() * 0.2), 
               mem: dwmMem * (0.9 + Math.random() * 0.2), 
               disk: 0, 
               net: 0 
             };

             if (openApps.length > 0) {
               const remainingCpu = payload.genesi_cpu * (1 - dwmWeight);
               const remainingMem = (payload.genesi_memory / (1024 * 1024)) * (1 - dwmWeight);
               const baseCpuPerApp = remainingCpu / openApps.length;
               const baseMemPerApp = remainingMem / openApps.length;
               
               openApps.forEach(app => {
                 // Adiciona uma variação aleatória de 10% para não ficarem todos com o mesmo número cravado
                 const cpuVar = baseCpuPerApp * (0.9 + Math.random() * 0.2);
                 const memVar = baseMemPerApp * (0.9 + Math.random() * 0.2);
                 next[app.id] = { cpu: cpuVar, mem: memVar, disk: 0, net: 0 };
               });
             }
             return next;
          });

        } catch (e) {
          console.error('Failed to get real processes:', e);
        }
      } else {
        // Fluctuate global mock
        setCpuUsage(prev => Math.max(1, Math.min(100, prev + (Math.random() * 10 - 5))));
        setMemUsage(prev => Math.max(10, Math.min(100, prev + (Math.random() * 2 - 1))));
        
        // Fluctuate processes mock
        setProcessMetrics(prev => {
          const next = { ...prev };
          Object.keys(next).forEach(key => {
            next[key] = {
              cpu: Math.max(0, Math.min(100, next[key].cpu + (Math.random() * 1 - 0.5))),
              mem: Math.max(10, next[key].mem + (Math.random() * 10 - 5)),
              disk: Math.max(0, next[key].disk + (Math.random() * 0.2 - 0.1)),
              net: Math.max(0, next[key].net + (Math.random() * 0.5 - 0.25))
            };
          });
          return next;
        });
      }
    };

    updateProcesses(); // initial call
    const interval = setInterval(updateProcesses, 2000);

    return () => clearInterval(interval);
  }, [apps]);

  const handleEndTask = async () => {
    if (selectedProcess) {
      const appExists = apps.find(a => a.id === selectedProcess);
      
      // Se for um app do GenesiOS, fecha ele nativamente pelo frontend
      if (appExists) {
        onCloseApp(selectedProcess);
        setSelectedProcess(null);
        return;
      }

      if (isTauri() && groupedProcesses.length > 0) {
        try {
          const group = groupedProcesses.find(g => g.name === selectedProcess);
          if (group && group.pids.length > 0) {
             const { Command } = await import('@tauri-apps/plugin-shell');
             // Mata todos os PIDs agrupados daquele app
             for (const pid of group.pids) {
                await Command.create('taskkill', ['/PID', pid.toString(), '/F']).execute().catch(() => {});
             }
             setSelectedProcess(null);
          }
        } catch (e) {
          console.error('Failed to kill process group', e);
        }
      } else {
        // Mock fallback logic
        setProcessMetrics(prev => {
          const next = { ...prev };
          delete next[selectedProcess];
          return next;
        });
        setSelectedProcess(null);
      }
    }
  };

  const getIconForProcess = (id: string) => {
    switch(id) {
      case 'files': return <FolderOpen size={16} className="text-yellow-400" />;
      case 'settings': return <Settings size={16} className="text-gray-400" />;
      case 'terminal': return <Terminal size={16} className="text-gray-300" />;
      case 'browser': return <Globe size={16} className="text-blue-400" />;
      case 'system': return <ShieldAlert size={16} className="text-red-400" />;
      case 'dwm': return <Box size={16} className="text-blue-300" />;
      case 'explorer': return <FolderOpen size={16} className="text-yellow-200" />;
      default: return <Box size={16} className="text-blue-500" />;
    }
  };

  const getProcessName = (id: string) => {
    const app = apps.find(a => a.id === id);
    if (app) return app.title || app.name;
    
    switch(id) {
      case 'system': return 'System Interrupts';
      case 'dwm': return 'Desktop Window Manager';
      case 'explorer': return 'Windows Explorer';
      default: return id;
    }
  };

  // Build the list of active processes (Real Apps + System Mocks)
  const activeProcesses = [
    ...apps.filter(a => a.isOpen).map(a => a.id),
    ...Object.keys(processMetrics).filter(k => !apps.some(a => a.id === k))
  ];

  return (
    <div className={`w-full h-full flex flex-col ${theme === 'light' ? 'bg-[#f3f3f3] text-black' : 'bg-[#202020] text-white'} overflow-hidden select-none`}>
      {/* HEADER / TOOLBAR */}
      <div className={`h-14 flex items-center justify-between px-4 border-b ${theme === 'light' ? 'border-black/10' : 'border-white/10'}`}>
        <div className="flex items-center gap-4">
          <div className="flex gap-2">
            <button 
              onClick={() => setActiveTab('processes')}
              className={`p-2 rounded-md flex flex-col items-center justify-center w-16 transition-colors ${activeTab === 'processes' ? (theme === 'light' ? 'bg-black/5' : 'bg-white/10') : 'hover:bg-white/5'}`}
            >
              <Activity size={20} strokeWidth={1.5} className={activeTab === 'processes' ? 'text-blue-500' : ''} />
            </button>
            <button 
              onClick={() => setActiveTab('performance')}
              className={`p-2 rounded-md flex flex-col items-center justify-center w-16 transition-colors ${activeTab === 'performance' ? (theme === 'light' ? 'bg-black/5' : 'bg-white/10') : 'hover:bg-white/5'}`}
            >
              <BarChart2 size={20} strokeWidth={1.5} className={activeTab === 'performance' ? 'text-blue-500' : ''} />
            </button>
          </div>
          <div className={`h-8 w-[1px] ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'}`}></div>
          <div className="relative">
            <Search size={14} className="absolute left-3 top-1/2 -translate-y-1/2 text-gray-400" />
            <input 
              type="text" 
              placeholder="Type to search" 
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className={`pl-9 pr-4 py-1.5 text-sm rounded-md outline-none w-64 ${theme === 'light' ? 'bg-white border border-black/10 text-black' : 'bg-black/40 border border-white/10 text-white focus:border-blue-500/50'}`}
            />
          </div>
        </div>
        <div className="flex gap-2">
          <button 
            className={`flex items-center gap-2 px-4 py-1.5 rounded-md text-sm transition-colors ${
              selectedProcess 
                ? (theme === 'light' ? 'bg-black/5 hover:bg-black/10 text-black' : 'bg-white/10 hover:bg-white/20 text-white') 
                : 'opacity-50 cursor-not-allowed'
            }`}
            onClick={handleEndTask}
            disabled={!selectedProcess}
          >
            <StopCircle size={16} className={selectedProcess ? 'text-red-500' : ''} />
            End task
          </button>
          <button className={`p-1.5 rounded-md ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}>
            <MoreHorizontal size={20} />
          </button>
        </div>
      </div>

      {/* CONTENT AREA */}
      <div className="flex-1 flex overflow-hidden">
        {/* SIDEBAR (Windows 11 Style) */}
        <div className={`w-14 flex flex-col items-center py-2 gap-2 border-r ${theme === 'light' ? 'border-black/10 bg-[#f9f9f9]' : 'border-white/10 bg-[#181818]'}`}>
           <div className="w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer relative group">
             <Activity size={20} className={activeTab === 'processes' ? 'text-blue-500' : 'text-gray-400'} />
             {activeTab === 'processes' && <div className="absolute left-0 w-1 h-4 bg-blue-500 rounded-r-full"></div>}
           </div>
           <div className="w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer relative group">
             <BarChart2 size={20} className={activeTab === 'performance' ? 'text-blue-500' : 'text-gray-400'} />
             {activeTab === 'performance' && <div className="absolute left-0 w-1 h-4 bg-blue-500 rounded-r-full"></div>}
           </div>
           <div className="w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer text-gray-400">
             <PlaySquare size={20} />
           </div>
           <div className="w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer text-gray-400">
             <User size={20} />
           </div>
           <div className="w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer text-gray-400">
             <Server size={20} />
           </div>
           <div className="mt-auto w-10 h-10 rounded-md flex items-center justify-center hover:bg-white/5 cursor-pointer text-gray-400">
             <Settings size={20} />
           </div>
        </div>

        {/* MAIN PANEL */}
        <div className="flex-1 overflow-y-auto">
          {activeTab === 'processes' && (
            <div className="w-full min-w-[600px]">
              {/* TABLE HEADER */}
              <div className={`grid grid-cols-[3fr_1fr_1fr_1fr_1fr] gap-4 px-6 py-2 text-xs border-b sticky top-0 z-10 ${theme === 'light' ? 'bg-[#f3f3f3] border-black/10' : 'bg-[#202020] border-white/10'}`}>
                <div className="font-medium flex items-center hover:bg-white/5 cursor-pointer rounded px-2 -ml-2">Name</div>
                <div className="font-medium flex items-center hover:bg-white/5 cursor-pointer rounded px-2 -ml-2">CPU</div>
                <div className="font-medium flex items-center hover:bg-white/5 cursor-pointer rounded px-2 -ml-2">Memory</div>
                <div className="font-medium flex items-center hover:bg-white/5 cursor-pointer rounded px-2 -ml-2">Disk</div>
                <div className="font-medium flex items-center hover:bg-white/5 cursor-pointer rounded px-2 -ml-2">Network</div>
              </div>
              
              {/* CPU/MEM BARS */}
              <div className={`grid grid-cols-[3fr_1fr_1fr_1fr_1fr] gap-4 px-6 py-2 text-xs border-b mb-2 ${theme === 'light' ? 'border-black/5' : 'border-white/5'}`}>
                <div></div>
                <div className="pr-4">
                  <div className="text-[10px] mb-1">{cpuUsage.toFixed(0)}%</div>
                  <div className={`w-full h-1 ${theme === 'light' ? 'bg-gray-300' : 'bg-gray-700'} rounded-full overflow-hidden`}>
                    <div className="h-full bg-blue-500" style={{ width: `${cpuUsage}%` }}></div>
                  </div>
                </div>
                <div className="pr-4">
                  <div className="text-[10px] mb-1">{memUsage.toFixed(0)}%</div>
                  <div className={`w-full h-1 ${theme === 'light' ? 'bg-gray-300' : 'bg-gray-700'} rounded-full overflow-hidden`}>
                    <div className="h-full bg-purple-500" style={{ width: `${memUsage}%` }}></div>
                  </div>
                </div>
                <div className="pr-4">
                  <div className="text-[10px] mb-1">0%</div>
                  <div className={`w-full h-1 ${theme === 'light' ? 'bg-gray-300' : 'bg-gray-700'} rounded-full overflow-hidden`}></div>
                </div>
                <div className="pr-4">
                  <div className="text-[10px] mb-1">0%</div>
                  <div className={`w-full h-1 ${theme === 'light' ? 'bg-gray-300' : 'bg-gray-700'} rounded-full overflow-hidden`}></div>
                </div>
              </div>

              {/* PROCESS LIST */}
              <div className="px-2 pb-4">
                {/* Genesi OS Apps Group */}
                <div className={`flex items-center gap-2 px-4 py-2 mt-4 text-xs font-semibold ${theme === 'light' ? 'text-gray-500' : 'text-gray-400'}`}>
                  <ChevronRight size={14} /> Genesi Apps ({apps.filter(a => a.isOpen).length})
                </div>
                {apps.filter(a => a.isOpen).map(app => {
                  const m = processMetrics[app.id] || { cpu: 0, mem: 0, disk: 0, net: 0 };
                  const isSelected = selectedProcess === app.id;
                  
                  return (
                    <div 
                      key={app.id}
                      onClick={() => setSelectedProcess(app.id)}
                      className={`grid grid-cols-[3fr_1fr_1fr_1fr_1fr] gap-4 px-4 py-2 text-sm items-center rounded-md cursor-default mb-0.5 ${
                        isSelected 
                          ? (theme === 'light' ? 'bg-blue-500/10' : 'bg-blue-500/20') 
                          : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5')
                      }`}
                    >
                      <div className="flex items-center gap-3 pl-6">
                        {getIconForProcess(app.id)}
                        <span>{app.title || app.name}</span>
                      </div>
                      <div className={`px-2 py-1 rounded ${m.cpu > 5 ? 'bg-[#ffe0e0] text-red-900 dark:bg-[#4a1c1c] dark:text-red-200' : ''}`}>{m.cpu.toFixed(1)}%</div>
                      <div className="px-2 py-1">{m.mem.toFixed(1)} MB</div>
                      <div className="px-2 py-1">{m.disk.toFixed(1)} MB/s</div>
                      <div className="px-2 py-1">{m.net.toFixed(1)} Mbps</div>
                    </div>
                  );
                })}

                {isTauri() && groupedProcesses.length > 0 ? (
                  <>
                    <div className={`flex items-center gap-2 px-4 py-2 mt-4 text-xs font-semibold ${theme === 'light' ? 'text-gray-500' : 'text-gray-400'}`}>
                      <ChevronRight size={14} /> System Processes ({groupedProcesses.length})
                    </div>
                    {groupedProcesses
                      .slice(0, 150) // limit render for perf
                      .map(p => {
                        // Use the first pid as an identifier or the group name
                        const isSelected = selectedProcess === p.name;
                        // O Windows Task Manager divide por 1024, mas sysinfo memory() traz o Resident Set Size (Working Set total).
                        // O Windows Task Manager mostra "Private Working Set".
                        // Uma conversão que se aproxima mais do visual do Windows Task Manager é dividir por (1024 * 1024) e aplicar um fator
                        // ou usar virtual_memory() no Rust. Por padrão, deixamos em MB (base 1024).
                        const memMB = p.memory / (1024 * 1024);
                        // Para não poluir, vamos arredondar pra 1 casa decimal como no Windows.
                        return (
                          <div 
                            key={p.name}
                            onClick={() => setSelectedProcess(p.name)}
                            className={`grid grid-cols-[3fr_1fr_1fr_1fr_1fr] gap-4 px-4 py-1.5 text-sm items-center rounded-md cursor-default mb-0.5 ${
                              isSelected 
                                ? (theme === 'light' ? 'bg-blue-500/10' : 'bg-blue-500/20') 
                                : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5')
                            }`}
                          >
                            <div className="flex items-center gap-3 pl-6 overflow-hidden">
                              <Box size={16} className="text-gray-400 shrink-0" />
                              <span className="truncate">{p.name} {p.pids.length > 1 ? `(${p.pids.length})` : ''}</span>
                            </div>
                            <div className={`px-2 py-1 rounded ${p.cpu > 5 ? 'bg-[#ffe0e0] text-red-900 dark:bg-[#4a1c1c] dark:text-red-200' : ''}`}>{p.cpu.toFixed(1)}%</div>
                            <div className="px-2 py-1">{memMB.toFixed(1)} MB</div>
                            <div className="px-2 py-1">0.0 MB/s</div>
                            <div className="px-2 py-1">0.0 Mbps</div>
                          </div>
                        );
                    })}
                  </>
                ) : (
                  <>
                {/* Background Processes Group */}
                <div className={`flex items-center gap-2 px-4 py-2 mt-4 text-xs font-semibold ${theme === 'light' ? 'text-gray-500' : 'text-gray-400'}`}>
                  <ChevronRight size={14} /> Background processes
                </div>
                {['dwm'].map(id => {
                  const m = processMetrics[id];
                  if (!m) return null;
                  const isSelected = selectedProcess === id;
                  
                  return (
                    <div 
                      key={id}
                      onClick={() => setSelectedProcess(id)}
                      className={`grid grid-cols-[3fr_1fr_1fr_1fr_1fr] gap-4 px-4 py-2 text-sm items-center rounded-md cursor-default mb-0.5 ${
                        isSelected 
                          ? (theme === 'light' ? 'bg-blue-500/10' : 'bg-blue-500/20') 
                          : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5')
                      }`}
                    >
                      <div className="flex items-center gap-3 pl-6">
                        {getIconForProcess(id)}
                        <span>{getProcessName(id)}</span>
                      </div>
                      <div className={`px-2 py-1 rounded ${m.cpu > 5 ? 'bg-[#ffe0e0] text-red-900 dark:bg-[#4a1c1c] dark:text-red-200' : ''}`}>{m.cpu.toFixed(1)}%</div>
                      <div className="px-2 py-1">{m.mem.toFixed(1)} MB</div>
                      <div className="px-2 py-1">{m.disk.toFixed(1)} MB/s</div>
                      <div className="px-2 py-1">{m.net.toFixed(1)} Mbps</div>
                    </div>
                  );
                })}
                </>
                )}
              </div>
            </div>
          )}

          {activeTab === 'performance' && (
            <div className="p-6 flex flex-col gap-6 w-full max-w-4xl">
              <h2 className="text-2xl font-semibold mb-2">Performance</h2>
              
              <div className="grid grid-cols-[1fr_3fr] gap-6">
                {/* Hardware List */}
                <div className="flex flex-col gap-2">
                  <div className={`p-3 rounded-lg border-l-4 border-blue-500 flex flex-col gap-1 cursor-pointer ${theme === 'light' ? 'bg-white shadow-sm' : 'bg-white/10'}`}>
                    <div className="flex justify-between items-center text-sm font-medium">
                      <div className="flex items-center gap-2"><Cpu size={16}/> CPU</div>
                      <span>{cpuUsage.toFixed(0)}%</span>
                    </div>
                    <div className="text-xs text-gray-400">AMD Ryzen 7 5800X</div>
                  </div>
                  <div className={`p-3 rounded-lg flex flex-col gap-1 cursor-pointer transition-colors ${theme === 'light' ? 'hover:bg-white' : 'hover:bg-white/5'}`}>
                    <div className="flex justify-between items-center text-sm font-medium">
                      <div className="flex items-center gap-2"><MemoryStick size={16}/> Memory</div>
                      <span>{memUsage.toFixed(0)}%</span>
                    </div>
                    <div className="text-xs text-gray-400">
                      {((totalMemory * (memUsage / 100)) / (1024 * 1024 * 1024)).toFixed(1)} / {(totalMemory / (1024 * 1024 * 1024)).toFixed(1)} GB
                    </div>
                  </div>
                  <div className={`p-3 rounded-lg flex flex-col gap-1 cursor-pointer transition-colors ${theme === 'light' ? 'hover:bg-white' : 'hover:bg-white/5'}`}>
                    <div className="flex justify-between items-center text-sm font-medium">
                      <div className="flex items-center gap-2"><HardDrive size={16}/> Disk 0 (C:)</div>
                      <span>1%</span>
                    </div>
                    <div className="text-xs text-gray-400">NVMe SSD</div>
                  </div>
                  <div className={`p-3 rounded-lg flex flex-col gap-1 cursor-pointer transition-colors ${theme === 'light' ? 'hover:bg-white' : 'hover:bg-white/5'}`}>
                    <div className="flex justify-between items-center text-sm font-medium">
                      <div className="flex items-center gap-2"><Wifi size={16}/> Wi-Fi</div>
                      <span>0 Kbps</span>
                    </div>
                    <div className="text-xs text-gray-400">Intel Wi-Fi 6 AX200</div>
                  </div>
                </div>

                {/* Graph Area */}
                <div className={`border rounded-lg p-6 flex flex-col gap-4 ${theme === 'light' ? 'bg-white border-black/10 shadow-sm' : 'bg-[#181818] border-white/10'}`}>
                  <div className="flex justify-between items-end">
                    <div>
                      <h3 className="text-xl font-medium mb-1">CPU</h3>
                      <p className="text-sm text-gray-400">% Utilization</p>
                    </div>
                    <h3 className="text-2xl font-light text-blue-500">{cpuUsage.toFixed(0)}%</h3>
                  </div>
                  
                  {/* Mock Graph */}
                  <div className="w-full h-48 border border-blue-500/20 rounded relative overflow-hidden flex items-end">
                    {/* Grid lines */}
                    <div className="absolute inset-0 grid grid-cols-10 grid-rows-5 gap-0 opacity-10">
                      {Array.from({ length: 50 }).map((_, i) => (
                        <div key={i} className="border-t border-l border-blue-500"></div>
                      ))}
                    </div>
                    {/* SVG Area chart mock */}
                    <svg className="w-full h-full absolute inset-0 text-blue-500" preserveAspectRatio="none" viewBox="0 0 100 100">
                      <polygon points="0,100 0,60 10,65 20,40 30,55 40,45 50,70 60,30 70,50 80,60 90,40 100,50 100,100" fill="currentColor" fillOpacity="0.1" stroke="currentColor" strokeWidth="1" vectorEffect="non-scaling-stroke" />
                    </svg>
                  </div>
                  
                  <div className="grid grid-cols-4 gap-4 mt-4">
                    <div>
                      <div className="text-xs text-gray-400 mb-1">Utilization</div>
                      <div className="text-lg">{cpuUsage.toFixed(0)}%</div>
                    </div>
                    <div>
                      <div className="text-xs text-gray-400 mb-1">Speed</div>
                      <div className="text-lg">4.20 GHz</div>
                    </div>
                    <div>
                      <div className="text-xs text-gray-400 mb-1">Processes</div>
                      <div className="text-lg">{activeProcesses.length + 152}</div>
                    </div>
                    <div>
                      <div className="text-xs text-gray-400 mb-1">Threads</div>
                      <div className="text-lg">2481</div>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default TaskManager;
