import React, { useState, useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { 
  IconFolderFilled, IconFile, IconDeviceFloppy, IconChevronRight, IconChevronLeft, IconChevronUp, 
  IconSearch, IconLayoutGrid, IconList, IconHome, IconPhoto, IconDownload, 
  IconDeviceDesktop, IconFileText, IconMusic, IconVideo, IconDots, IconArrowUp, IconRefresh, 
  IconPlus, IconCut, IconCopy, IconClipboard, IconEdit, IconTrash
} from '@tabler/icons-react';

interface DiskInfo {
  name: string;
  mount_point: string;
  total_space: number;
  available_space: number;
}

interface FileInfo {
  name: string;
  path: string;
  is_dir: boolean;
  size: number;
}

const FileExplorer = () => {
  const [currentPath, setCurrentPath] = useState<string>('Home');
  const [history, setHistory] = useState<string[]>(['Home']);
  const [historyIndex, setHistoryIndex] = useState<number>(0);
  
  const [drives, setDrives] = useState<DiskInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [viewMode, setViewMode] = useState<'grid' | 'list'>('grid');

  useEffect(() => {
    loadDrives();
  }, []);

  useEffect(() => {
    if (currentPath !== 'Home') {
      loadFiles(currentPath);
    }
  }, [currentPath]);

  const loadDrives = async () => {
    try {
      const result: DiskInfo[] = await invoke('get_drives');
      setDrives(result);
    } catch (e) {
      console.error('Error loading drives:', e);
    }
  };

  const loadFiles = async (path: string) => {
    setLoading(true);
    try {
      const result: FileInfo[] = await invoke('read_dir', { path });
      setFiles(result);
    } catch (e) {
      console.error('Error reading dir:', e);
      setFiles([]);
    } finally {
      setLoading(false);
    }
  };

  const navigateTo = (path: string) => {
    if (path === currentPath) return;
    const newHistory = history.slice(0, historyIndex + 1);
    newHistory.push(path);
    setHistory(newHistory);
    setHistoryIndex(newHistory.length - 1);
    setCurrentPath(path);
  };

  const goBack = () => {
    if (historyIndex > 0) {
      setHistoryIndex(historyIndex - 1);
      setCurrentPath(history[historyIndex - 1]);
    }
  };

  const goForward = () => {
    if (historyIndex < history.length - 1) {
      setHistoryIndex(historyIndex + 1);
      setCurrentPath(history[historyIndex + 1]);
    }
  };

  const goUp = () => {
    if (currentPath === 'Home') return;
    
    const parts = currentPath.split(/[\\/]/).filter(p => p);
    if (parts.length <= 1) {
      navigateTo('Home');
    } else {
      parts.pop();
      const newPath = currentPath.includes('\\') 
        ? parts.join('\\') + (parts.length === 1 ? '\\' : '')
        : '/' + parts.join('/');
      navigateTo(newPath);
    }
  };

  const formatSize = (bytes: number) => {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const formatDriveName = (name: string, mountPoint: string) => {
    const driveLetter = mountPoint.replace(/[\\/]$/, ''); // Remove trailing slash
    const volName = name.trim() === '' ? 'Local Disk' : name;
    return `${volName} (${driveLetter})`;
  };

  const quickAccess = [
    { icon: IconHome, label: 'Home', path: 'Home', color: 'text-blue-400' },
    { icon: IconDeviceDesktop, label: 'Desktop', path: '', color: 'text-cyan-400' },
    { icon: IconDownload, label: 'Downloads', path: '', color: 'text-green-400' },
    { icon: IconFileText, label: 'Documents', path: '', color: 'text-yellow-400' },
    { icon: IconPhoto, label: 'Pictures', path: '', color: 'text-purple-400' },
    { icon: IconMusic, label: 'Music', path: '', color: 'text-pink-400' },
    { icon: IconVideo, label: 'Videos', path: '', color: 'text-orange-400' },
  ];

  return (
    <div className="flex flex-col w-full h-full bg-[#1e1e1e] text-white/90 overflow-hidden font-sans rounded-b-xl border border-white/5">
      
      {/* 1. Command Bar (Ribbon) */}
      <div className="h-[48px] bg-[#2d2d2d] border-b border-white/5 flex items-center px-4 gap-2 shrink-0">
        <button className="flex items-center gap-2 px-3 py-1.5 hover:bg-white/10 rounded-md transition-colors text-[13px] font-medium">
          <IconPlus size={16} /> New
        </button>
        <div className="w-[1px] h-6 bg-white/10 mx-1"></div>
        <button className="p-2 hover:bg-white/10 rounded-md transition-colors" title="Cut"><IconCut size={16} className="text-white/80" /></button>
        <button className="p-2 hover:bg-white/10 rounded-md transition-colors" title="Copy"><IconCopy size={16} className="text-white/80" /></button>
        <button className="p-2 hover:bg-white/10 rounded-md transition-colors" title="Paste"><IconClipboard size={16} className="text-white/80" /></button>
        <button className="p-2 hover:bg-white/10 rounded-md transition-colors" title="Rename"><IconEdit size={16} className="text-white/80" /></button>
        <button className="p-2 hover:bg-white/10 rounded-md transition-colors" title="Delete"><IconTrash size={16} className="text-red-400" /></button>
        <div className="w-[1px] h-6 bg-white/10 mx-1"></div>
        
        <div className="ml-auto flex items-center gap-1">
          <button 
            className={`p-2 rounded-md transition-colors ${viewMode === 'list' ? 'bg-white/10' : 'hover:bg-white/10'}`}
            onClick={() => setViewMode('list')}
          >
            <IconList size={16} className="text-white/80" />
          </button>
          <button 
            className={`p-2 rounded-md transition-colors ${viewMode === 'grid' ? 'bg-white/10' : 'hover:bg-white/10'}`}
            onClick={() => setViewMode('grid')}
          >
            <IconLayoutGrid size={16} className="text-white/80" />
          </button>
          <button className="p-2 hover:bg-white/10 rounded-md transition-colors ml-1">
            <IconDots size={16} className="text-white/80" />
          </button>
        </div>
      </div>

      {/* 2. Address Bar */}
      <div className="h-[48px] bg-[#1e1e1e] border-b border-white/5 flex items-center px-4 gap-3 shrink-0">
        <div className="flex items-center gap-1">
          <button 
            className={`p-1.5 rounded-md transition-colors ${historyIndex > 0 ? 'hover:bg-white/10 text-white' : 'text-white/30 cursor-not-allowed'}`}
            onClick={goBack}
          >
            <IconChevronLeft size={20} stroke={1.5} />
          </button>
          <button 
            className={`p-1.5 rounded-md transition-colors ${historyIndex < history.length - 1 ? 'hover:bg-white/10 text-white' : 'text-white/30 cursor-not-allowed'}`}
            onClick={goForward}
          >
            <IconChevronRight size={20} stroke={1.5} />
          </button>
          <button 
            className={`p-1.5 rounded-md transition-colors ${currentPath !== 'Home' ? 'hover:bg-white/10 text-white' : 'text-white/30 cursor-not-allowed'}`}
            onClick={goUp}
          >
            <IconArrowUp size={18} stroke={1.5} />
          </button>
          <button className="p-1.5 rounded-md hover:bg-white/10 transition-colors" onClick={() => currentPath === 'Home' ? loadDrives() : loadFiles(currentPath)}>
            <IconRefresh size={16} stroke={1.5} className="text-white/80" />
          </button>
        </div>

        <div className="flex-1 flex items-center bg-[#2d2d2d] border border-white/10 rounded-md px-3 py-1.5 focus-within:border-blue-500/50 transition-colors">
          {currentPath === 'Home' ? (
            <IconHome size={16} className="text-blue-400 mr-2 shrink-0" stroke={1.5} />
          ) : (
            <IconFolderFilled size={16} className="text-yellow-500 mr-2 shrink-0" />
          )}
          <input 
            type="text" 
            value={currentPath} 
            onChange={(e) => setCurrentPath(e.target.value)}
            onKeyDown={(e) => e.key === 'Enter' && navigateTo(currentPath)}
            className="bg-transparent border-none outline-none w-full text-[13px]"
          />
        </div>

        <div className="w-[250px] relative flex items-center bg-[#2d2d2d] border border-white/10 rounded-md px-3 py-1.5 focus-within:border-blue-500/50 transition-colors">
          <input 
            type="text" 
            placeholder="Search" 
            className="bg-transparent border-none outline-none w-full text-[13px]"
          />
          <IconSearch size={16} className="text-white/40 ml-2 shrink-0" stroke={1.5} />
        </div>
      </div>

      {/* 3. Main Content Area (Sidebar + File View) */}
      <div className="flex-1 flex overflow-hidden">
        
        {/* Navigation Pane (Sidebar) */}
        <div className="w-[220px] bg-[#1e1e1e] border-r border-white/5 overflow-y-auto custom-scrollbar flex flex-col py-2 shrink-0">
          
          {/* Quick Access */}
          <div className="px-2 mb-4">
            <div className="px-3 py-1 text-[11px] font-semibold text-white/50 mb-1">Quick Access</div>
            {quickAccess.map((item, i) => (
              <div 
                key={i} 
                onClick={() => item.path ? navigateTo(item.path) : null}
                className={`flex items-center gap-3 px-3 py-1.5 rounded-md cursor-pointer transition-colors ${currentPath === item.path ? 'bg-white/10' : 'hover:bg-white/5'}`}
              >
                <item.icon size={16} stroke={1.5} className={item.color} />
                <span className="text-[13px]">{item.label}</span>
              </div>
            ))}
          </div>

          {/* This PC (Drives) */}
          <div className="px-2">
            <div className="px-3 py-1 text-[11px] font-semibold text-white/50 mb-1 flex items-center justify-between group cursor-pointer hover:text-white/80" onClick={() => navigateTo('Home')}>
              This PC <IconChevronUp size={14} className="opacity-0 group-hover:opacity-100" />
            </div>
            {drives.map((drive, i) => (
              <div 
                key={i} 
                onClick={() => navigateTo(drive.mount_point)}
                className={`flex items-center gap-3 px-3 py-1.5 rounded-md cursor-pointer transition-colors ${currentPath === drive.mount_point ? 'bg-white/10' : 'hover:bg-white/5'}`}
              >
                <IconDeviceFloppy size={16} stroke={1.5} className="text-gray-300" />
                <span className="text-[13px] truncate">{formatDriveName(drive.name, drive.mount_point)}</span>
              </div>
            ))}
          </div>

        </div>

        {/* File View Area */}
        <div className="flex-1 bg-[#191919] overflow-y-auto custom-scrollbar relative">
          
          {loading && (
            <div className="absolute inset-0 flex items-center justify-center bg-[#191919]/50 z-10">
              <div className="animate-spin w-8 h-8 border-2 border-blue-500 border-t-transparent rounded-full"></div>
            </div>
          )}

          {currentPath === 'Home' ? (
            <div className="p-6">
              <h2 className="text-lg font-semibold mb-4">Drives</h2>
              <div className="grid grid-cols-1 xl:grid-cols-2 2xl:grid-cols-3 gap-4">
                {drives.map((drive, i) => {
                  const usedSpace = drive.total_space > 0 ? drive.total_space - drive.available_space : 0;
                  const usagePercent = drive.total_space > 0 ? (usedSpace / drive.total_space) * 100 : 0;
                  const isFull = usagePercent > 90;

                  return (
                    <div 
                      key={i} 
                      onClick={() => navigateTo(drive.mount_point)}
                      className="flex items-center gap-4 bg-white/5 hover:bg-white/10 border border-white/5 p-4 rounded-xl cursor-pointer transition-all hover:-translate-y-1"
                    >
                      <IconDeviceFloppy size={44} stroke={1.5} className="text-blue-400 shrink-0" />
                      <div className="flex flex-col w-full">
                        <div className="flex items-center justify-between mb-1">
                          <span className="font-semibold text-[13px]">{formatDriveName(drive.name, drive.mount_point)}</span>
                        </div>
                        
                        {drive.total_space > 0 ? (
                          <>
                            <div className="w-full h-[6px] bg-black/40 rounded-full overflow-hidden mb-1 border border-white/5">
                              <div 
                                className={`h-full rounded-full ${isFull ? 'bg-red-500' : 'bg-blue-500'}`}
                                style={{ width: `${usagePercent}%` }}
                              ></div>
                            </div>
                            <span className="text-[11px] text-white/50">
                              {formatSize(drive.available_space)} free of {formatSize(drive.total_space)}
                            </span>
                          </>
                        ) : (
                          <span className="text-[11px] text-white/50">Unknown space</span>
                        )}
                      </div>
                    </div>
                  );
                })}
              </div>
            </div>
          ) : (
            <div className="p-2">
              {viewMode === 'grid' ? (
                <div className="grid grid-cols-[repeat(auto-fill,minmax(100px,1fr))] gap-2 p-2">
                  {files.map((file, i) => (
                    <div 
                      key={i} 
                      onClick={() => file.is_dir ? navigateTo(file.path) : null}
                      className="flex flex-col items-center gap-2 p-3 rounded-lg hover:bg-white/10 cursor-pointer transition-colors text-center group"
                    >
                      {file.is_dir ? (
                        <IconFolderFilled size={48} className="text-yellow-500 group-hover:scale-105 transition-transform" />
                      ) : (
                        <IconFile size={48} stroke={1.5} className="text-white/80 group-hover:scale-105 transition-transform" />
                      )}
                      <span className="text-[12px] break-words w-full line-clamp-2 leading-tight" title={file.name}>{file.name}</span>
                    </div>
                  ))}
                </div>
              ) : (
                <table className="w-full text-left border-collapse">
                  <thead>
                    <tr className="border-b border-white/5 text-[12px] text-white/50">
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer">Name</th>
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer w-32">Type</th>
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer w-32">Size</th>
                    </tr>
                  </thead>
                  <tbody>
                    {files.map((file, i) => (
                      <tr 
                        key={i} 
                        onClick={() => file.is_dir ? navigateTo(file.path) : null}
                        className="hover:bg-white/5 cursor-pointer border-b border-transparent hover:border-white/5 text-[13px] transition-colors"
                      >
                        <td className="px-4 py-2 flex items-center gap-3">
                          {file.is_dir ? <IconFolderFilled size={16} className="text-yellow-500" /> : <IconFile size={16} stroke={1.5} className="text-white/60" />}
                          <span className="truncate" title={file.name}>{file.name}</span>
                        </td>
                        <td className="px-4 py-2 text-white/50">{file.is_dir ? 'File folder' : 'File'}</td>
                        <td className="px-4 py-2 text-white/50">{file.is_dir ? '' : formatSize(file.size)}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              )}
              {files.length === 0 && !loading && (
                <div className="flex flex-col items-center justify-center py-20 text-white/30">
                  <IconFolderFilled size={64} className="mb-4 opacity-50" />
                  <p>This folder is empty.</p>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
      
      {/* Status Bar */}
      <div className="h-[24px] bg-[#0078D7] text-white flex items-center px-4 shrink-0 text-[11px]">
        {currentPath === 'Home' 
          ? `${drives.length} item(s)` 
          : `${files.length} item(s)`
        }
      </div>

    </div>
  );
};

export default FileExplorer;