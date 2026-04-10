import React, { useState, useEffect } from 'react';
import { invoke } from '@tauri-apps/api/core';
import { LazyStore } from '@tauri-apps/plugin-store';
import { 
  IconFolderFilled, IconFile, IconDeviceFloppy, IconChevronRight, IconChevronLeft, IconChevronUp, IconChevronDown,
  IconSearch, IconLayoutGrid, IconList, IconHome, IconPhoto, IconDownload, 
  IconDeviceDesktop, IconFileText, IconMusic, IconVideo, IconDots, IconArrowUp, IconRefresh, 
  IconPlus, IconCut, IconCopy, IconClipboard, IconEdit, IconTrash
} from '@tabler/icons-react';

const store = new LazyStore('settings.json');

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
  modified_at: number;
}

export const FileExplorerBase = ({ isPicker = false, onFileSelect, onClosePicker }: { isPicker?: boolean, onFileSelect?: (url: string, path?: string) => void, onClosePicker?: () => void }) => {
  const [currentPath, setCurrentPath] = useState<string>('Home');
  const [history, setHistory] = useState<string[]>(['Home']);
  const [historyIndex, setHistoryIndex] = useState<number>(0);
  
  const [drives, setDrives] = useState<DiskInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [viewMode, setViewModeState] = useState<'grid' | 'list'>('list');

  // Filtering and Sorting state
  const [searchQuery, setSearchQuery] = useState('');
  const [typeFilter, setTypeFilterState] = useState('all');
  const [sortBy, setSortByState] = useState<'name' | 'size' | 'date' | 'type'>('name');
  const [sortOrder, setSortOrderState] = useState<'asc' | 'desc'>('asc');

  // Load persisted state
  useEffect(() => {
    const loadState = async () => {
      try {
        const savedViewMode = await store.get<'grid' | 'list'>('explorer_view_mode');
        if (savedViewMode) setViewModeState(savedViewMode);

        const savedTypeFilter = await store.get<string>('explorer_type_filter');
        if (savedTypeFilter) setTypeFilterState(savedTypeFilter);

        const savedSortBy = await store.get<'name' | 'size' | 'date' | 'type'>('explorer_sort_by');
        if (savedSortBy) setSortByState(savedSortBy);

        const savedSortOrder = await store.get<'asc' | 'desc'>('explorer_sort_order');
        if (savedSortOrder) setSortOrderState(savedSortOrder);
      } catch (error) {
        console.error('Failed to load explorer state:', error);
      }
    };
    loadState();
  }, []);

  const handleViewModeChange = async (mode: 'grid' | 'list') => {
    setViewModeState(mode);
    await store.set('explorer_view_mode', mode);
    await store.save();
  };

  const handleTypeFilterChange = async (filter: string) => {
    setTypeFilterState(filter);
    await store.set('explorer_type_filter', filter);
    await store.save();
  };

  const getFileExtension = (filename: string) => {
    const parts = filename.split('.');
    return parts.length > 1 ? parts.pop()?.toLowerCase() || '' : '';
  };

  const getFileTypeCategory = (filename: string) => {
    if (!filename.includes('.')) return 'other';
    const ext = getFileExtension(filename);
    if (['jpg', 'jpeg', 'png', 'gif', 'webp', 'bmp', 'svg'].includes(ext)) return 'image';
    if (['mp4', 'mkv', 'avi', 'mov', 'webm'].includes(ext)) return 'video';
    if (['mp3', 'wav', 'ogg', 'flac'].includes(ext)) return 'audio';
    if (['pdf', 'doc', 'docx', 'txt', 'rtf', 'md', 'csv', 'xlsx'].includes(ext)) return 'document';
    if (['zip', 'rar', '7z', 'tar', 'gz'].includes(ext)) return 'archive';
    return 'other';
  };

  const filteredAndSortedFiles = React.useMemo(() => {
    let result = [...files];

    if (searchQuery) {
      result = result.filter(f => f.name.toLowerCase().includes(searchQuery.toLowerCase()));
    }

    if (typeFilter !== 'all') {
      result = result.filter(f => {
        if (f.is_dir) return true;
        return getFileTypeCategory(f.name) === typeFilter;
      });
    }

    result.sort((a, b) => {
      if (a.is_dir !== b.is_dir) {
        return a.is_dir ? -1 : 1;
      }

      let comparison = 0;
      switch (sortBy) {
        case 'name':
          comparison = a.name.localeCompare(b.name);
          break;
        case 'size':
          comparison = a.size - b.size;
          break;
        case 'date':
          comparison = a.modified_at - b.modified_at;
          break;
        case 'type':
          if (a.is_dir) {
            comparison = a.name.localeCompare(b.name);
          } else {
            const extA = getFileExtension(a.name);
            const extB = getFileExtension(b.name);
            comparison = extA.localeCompare(extB) || a.name.localeCompare(b.name);
          }
          break;
      }

      return sortOrder === 'asc' ? comparison : -comparison;
    });

    return result;
  }, [files, searchQuery, typeFilter, sortBy, sortOrder]);

  const handleSort = async (field: 'name' | 'size' | 'date' | 'type') => {
    try {
      if (sortBy === field) {
        const newOrder = sortOrder === 'asc' ? 'desc' : 'asc';
        setSortOrderState(newOrder);
        await store.set('explorer_sort_order', newOrder);
        await store.save();
      } else {
        setSortByState(field);
        setSortOrderState('asc');
        await store.set('explorer_sort_by', field);
        await store.set('explorer_sort_order', 'asc');
        await store.save();
      }
    } catch (e) {
      console.error('Failed to save sort state', e);
    }
  };

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

  const handleFileClick = async (file: FileInfo) => {
    if (file.is_dir) {
      navigateTo(file.path);
    } else if (isPicker && onFileSelect) {
      const isImage = file.name.match(/\.(jpg|jpeg|png|gif|webp)$/i);
      if (isImage) {
        try {
          const bytes: number[] = await invoke('read_file_bytes', { path: file.path });
          const blob = new Blob([new Uint8Array(bytes)]);
          const url = URL.createObjectURL(blob);
          onFileSelect(url, file.path);
        } catch (e) {
          console.error('Failed to load image', e);
        }
      }
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
    <div className={`flex flex-col w-full h-full bg-[#1e1e1e] text-white/90 overflow-hidden font-sans ${isPicker ? '' : 'rounded-b-xl border border-white/5'}`}>
      
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
            onClick={() => handleViewModeChange('list')}
          >
            <IconList size={16} className="text-white/80" />
          </button>
          <button 
            className={`p-2 rounded-md transition-colors ${viewMode === 'grid' ? 'bg-white/10' : 'hover:bg-white/10'}`}
            onClick={() => handleViewModeChange('grid')}
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

        <div className="flex gap-2">
          <select
            value={typeFilter}
            onChange={(e) => handleTypeFilterChange(e.target.value)}
            className="bg-[#2d2d2d] border border-white/10 rounded-md px-2 py-1.5 text-[13px] outline-none focus:border-blue-500/50 transition-colors cursor-pointer"
          >
            <option value="all">All Types</option>
            <option value="image">Images</option>
            <option value="video">Videos</option>
            <option value="audio">Audio</option>
            <option value="document">Documents</option>
            <option value="archive">Archives</option>
          </select>

          <div className="w-[200px] relative flex items-center bg-[#2d2d2d] border border-white/10 rounded-md px-3 py-1.5 focus-within:border-blue-500/50 transition-colors">
            <input 
              type="text" 
              placeholder="Search" 
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="bg-transparent border-none outline-none w-full text-[13px]"
            />
            <IconSearch size={16} className="text-white/40 ml-2 shrink-0" stroke={1.5} />
          </div>
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
                  {filteredAndSortedFiles.map((file, i) => {
                    const isImage = file.name.match(/\.(jpg|jpeg|png|gif|webp)$/i);
                    return (
                    <div 
                      key={i} 
                      onClick={() => handleFileClick(file)}
                      className="flex flex-col items-center gap-2 p-3 rounded-lg hover:bg-white/10 cursor-pointer transition-colors text-center group"
                    >
                      {file.is_dir ? (
                        <IconFolderFilled size={48} className="text-yellow-500 group-hover:scale-105 transition-transform" />
                      ) : isImage && isPicker ? (
                        <IconPhoto size={48} className="text-purple-400 group-hover:scale-105 transition-transform" />
                      ) : (
                        <IconFile size={48} stroke={1.5} className="text-white/80 group-hover:scale-105 transition-transform" />
                      )}
                      <span className="text-[12px] break-words w-full line-clamp-2 leading-tight" title={file.name}>{file.name}</span>
                    </div>
                  )})}
                </div>
              ) : (
                <table className="w-full text-left border-collapse table-fixed">
                  <thead className="sticky top-0 bg-[#191919] z-10 shadow-[0_1px_0_rgba(255,255,255,0.05)]">
                    <tr className="text-[12px] text-white/70 select-none">
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer transition-colors" onClick={() => handleSort('name')}>
                        <div className="flex items-center gap-2">
                          Name {sortBy === 'name' && (sortOrder === 'asc' ? <IconChevronUp size={14} className="text-white shrink-0" /> : <IconChevronDown size={14} className="text-white shrink-0" />)}
                        </div>
                      </th>
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer w-40 transition-colors" onClick={() => handleSort('date')}>
                        <div className="flex items-center gap-2">
                          Date modified {sortBy === 'date' && (sortOrder === 'asc' ? <IconChevronUp size={14} className="text-white shrink-0" /> : <IconChevronDown size={14} className="text-white shrink-0" />)}
                        </div>
                      </th>
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer w-32 transition-colors" onClick={() => handleSort('type')}>
                        <div className="flex items-center gap-2">
                          Type {sortBy === 'type' && (sortOrder === 'asc' ? <IconChevronUp size={14} className="text-white shrink-0" /> : <IconChevronDown size={14} className="text-white shrink-0" />)}
                        </div>
                      </th>
                      <th className="font-normal px-4 py-2 hover:bg-white/5 cursor-pointer w-32 transition-colors" onClick={() => handleSort('size')}>
                        <div className="flex items-center gap-2">
                          Size {sortBy === 'size' && (sortOrder === 'asc' ? <IconChevronUp size={14} className="text-white shrink-0" /> : <IconChevronDown size={14} className="text-white shrink-0" />)}
                        </div>
                      </th>
                    </tr>
                  </thead>
                  <tbody>
                    {filteredAndSortedFiles.map((file, i) => (
                      <tr 
                        key={i} 
                        onClick={() => handleFileClick(file)}
                        className="hover:bg-white/5 cursor-pointer border-b border-transparent hover:border-white/5 text-[13px] transition-colors"
                      >
                        <td className="px-4 py-2">
                          <div className="flex items-center gap-3 w-full overflow-hidden">
                            <div className="shrink-0 flex items-center justify-center">
                              {file.is_dir ? <IconFolderFilled size={16} className="text-yellow-500" /> : <IconFile size={16} stroke={1.5} className="text-white/60" />}
                            </div>
                            <span className="truncate" title={file.name}>{file.name}</span>
                          </div>
                        </td>
                        <td className="px-4 py-2 text-white/50 truncate">{file.modified_at ? new Date(file.modified_at * 1000).toLocaleDateString() : ''}</td>
                        <td className="px-4 py-2 text-white/50 truncate">{file.is_dir ? 'File folder' : getFileExtension(file.name).toUpperCase() + ' File'}</td>
                        <td className="px-4 py-2 text-white/50 truncate">{file.is_dir ? '' : formatSize(file.size)}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              )}
              {filteredAndSortedFiles.length === 0 && !loading && (
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
          : `${filteredAndSortedFiles.length} item(s)`
        }
      </div>

    </div>
  );
};

export default function FileExplorer() {
  return <FileExplorerBase />;
}