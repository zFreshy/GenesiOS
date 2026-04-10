import React, { useState, useEffect } from 'react';
import { createPortal } from 'react-dom';
import { invoke } from '@tauri-apps/api/core';
import { LazyStore } from '@tauri-apps/plugin-store';
import { openPath } from '@tauri-apps/plugin-opener';
import { open } from '@tauri-apps/plugin-dialog';
import { useTheme } from './ThemeContext';
import { 
  IconFolderFilled, IconFile, IconDeviceFloppy, IconChevronRight, IconChevronLeft, IconChevronUp, IconChevronDown,
  IconSearch, IconLayoutGrid, IconList, IconHome, IconPhoto, IconDownload, 
  IconDeviceDesktop, IconFileText, IconMusic, IconVideo, IconDots, IconArrowUp, IconRefresh, 
  IconPlus, IconCut, IconCopy, IconClipboard, IconEdit, IconTrash, IconX
} from '@tabler/icons-react';

import ImageViewer from './ImageViewer';
import VideoPlayer from './VideoPlayer';
import TextEditor from './TextEditor';

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

export const FileExplorerBase = ({ isPicker = false, pickerMode = 'file', onFileSelect, onFolderSelect, onClosePicker, onOpenInApp }: { isPicker?: boolean, pickerMode?: 'file' | 'folder', onFileSelect?: (url: string, path?: string) => void, onFolderSelect?: (path: string) => void, onClosePicker?: () => void, onOpenInApp?: (baseId: string, props: any) => void }) => {
  if (false) console.log(onClosePicker); // bypass unused warning
  const { theme } = useTheme();
  const [currentPath, setCurrentPath] = useState<string>('Home');
  const [history, setHistory] = useState<string[]>(['Home']);
  const [historyIndex, setHistoryIndex] = useState<number>(0);
  
  const [drives, setDrives] = useState<DiskInfo[]>([]);
  const [files, setFiles] = useState<FileInfo[]>([]);
  const [loading, setLoading] = useState<boolean>(false);
  const [viewMode, setViewModeState] = useState<'grid' | 'list'>('list');
  const [selectedFile, setSelectedFile] = useState<string | null>(null);

  // Context Menu
  const [contextMenu, setContextMenu] = useState<{ x: number, y: number, type: 'file' | 'folder' | 'quick_access', item: any } | null>(null);
  
  // Folder Picker Target
  const [folderPickerTarget, setFolderPickerTarget] = useState<any | null>(null);

  // Quick Access state
  const defaultQuickAccess = [
    { id: 'home', icon: IconHome, label: 'Home', path: 'Home', color: 'text-blue-400', isSystem: true },
    { id: 'desktop', icon: IconDeviceDesktop, label: 'Desktop', path: '', color: 'text-cyan-400', isSystem: true },
    { id: 'downloads', icon: IconDownload, label: 'Downloads', path: '', color: 'text-green-400', isSystem: true },
    { id: 'documents', icon: IconFileText, label: 'Documents', path: '', color: 'text-yellow-400', isSystem: true },
    { id: 'pictures', icon: IconPhoto, label: 'Pictures', path: '', color: 'text-purple-400', isSystem: true },
    { id: 'music', icon: IconMusic, label: 'Music', path: '', color: 'text-pink-400', isSystem: true },
    { id: 'videos', icon: IconVideo, label: 'Videos', path: '', color: 'text-orange-400', isSystem: true },
  ];
  const [quickAccess, setQuickAccess] = useState<any[]>(defaultQuickAccess);

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

        // Obter caminhos padrão do sistema via Rust
        const sysPaths = await invoke<Record<string, string>>('get_default_paths').catch((e) => {
          console.error("Failed to get default paths from Rust (make sure backend is updated):", e);
          return {};
        });
        
        // Mesclar com o estado padrão
        const baseQuickAccess = defaultQuickAccess.map(qa => ({
          ...qa,
          path: sysPaths[qa.id] || qa.path
        }));

        const savedQuickAccess = await store.get<any[]>('explorer_quick_access');
        let finalQuickAccess = baseQuickAccess;

        if (savedQuickAccess && Array.isArray(savedQuickAccess)) {
          finalQuickAccess = savedQuickAccess.map(sqa => {
            const def = baseQuickAccess.find(d => d.id === sqa.id);
            if (def) {
               // Se estiver vazio ou for uma string em branco, usa o padrão do sistema
               const validPath = (sqa.path && sqa.path.trim() !== '') ? sqa.path : def.path;
               return { ...def, path: validPath };
            }
            return { ...sqa, icon: IconFolderFilled, color: 'text-yellow-400' };
          });
        }
        
        setQuickAccess(finalQuickAccess);
        // Atualiza a loja com os caminhos consertados para não precisar recalcular ou caso estivessem vazios
        await store.set('explorer_quick_access', finalQuickAccess);
        await store.save();

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

  const handleFileClick = (file: FileInfo) => {
    setSelectedFile(file.path);
  };

  const handleFileDoubleClick = async (file: FileInfo) => {
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
    } else if (!isPicker) {
      // Tentar abrir dentro do Genesi se for formato suportado
      const isImage = file.name.match(/\.(jpg|jpeg|png|gif|webp)$/i);
      const isVideo = file.name.match(/\.(mp4|webm|ogg)$/i);
      const isText = file.name.match(/\.(txt|md|json|js|ts|jsx|tsx|css|html|xml)$/i);

      if (isImage && onOpenInApp) {
        onOpenInApp('image-viewer', {
          title: `Photos - ${file.name}`,
          content: <ImageViewer filePath={file.path} fileName={file.name} />
        });
      } else if (isVideo && onOpenInApp) {
        onOpenInApp('video-player', {
          title: `Video Player - ${file.name}`,
          content: <VideoPlayer filePath={file.path} fileName={file.name} />
        });
      } else if (isText && onOpenInApp) {
        onOpenInApp('text-editor', {
          title: `Text Editor - ${file.name}`,
          content: <TextEditor filePath={file.path} fileName={file.name} />
        });
      } else {
        // Se não suportado internamente, tenta abrir no PC
        try {
          await openPath(file.path);
        } catch (e) {
          console.error('Failed to open file natively', e);
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

  useEffect(() => {
    const handleClickOutside = () => setContextMenu(null);
    window.addEventListener('click', handleClickOutside);
    return () => window.removeEventListener('click', handleClickOutside);
  }, []);

  const handlePinToQuickAccess = async (folder: FileInfo) => {
    const newQuickAccess = [...quickAccess, {
      id: `custom_${Date.now()}`,
      icon: IconFolderFilled,
      label: folder.name,
      path: folder.path,
      color: 'text-yellow-400',
      isSystem: false
    }];
    setQuickAccess(newQuickAccess);
    await store.set('explorer_quick_access', newQuickAccess);
    await store.save();
  };

  const handleUnpinQuickAccess = async (item: any) => {
    const newQuickAccess = quickAccess.filter(q => q.id !== item.id);
    setQuickAccess(newQuickAccess);
    await store.set('explorer_quick_access', newQuickAccess);
    await store.save();
  };

  const handleChangeQuickAccessPath = async (item: any) => {
    setFolderPickerTarget(item);
  };

  const handleCreateDesktopShortcut = async (file: FileInfo) => {
    try {
      // Remove extension for shortcut name if desired, or keep it
      const shortcutName = file.name.replace(/\.[^/.]+$/, "");
      await invoke('create_desktop_shortcut', { 
        targetPath: file.path, 
        fileName: shortcutName 
      });
      console.log('Shortcut created successfully');
    } catch (error) {
      console.error('Failed to create shortcut', error);
    }
  };

  return (
    <div 
      className={`flex flex-col w-full h-full ${theme === 'light' ? 'bg-[#f5f5f5] text-black/90' : 'bg-[#1e1e1e] text-white/90'} overflow-hidden font-sans ${isPicker ? '' : `rounded-b-xl border ${theme === 'light' ? 'border-black/5' : 'border-white/5'}`}`}
      onMouseUp={(e) => {
        if (e.button === 3) {
          e.preventDefault();
          goBack();
        } else if (e.button === 4) {
          e.preventDefault();
          goForward();
        }
      }}
    >
      
      {/* 1. Command Bar (Ribbon) */}
      <div className={`h-[48px] ${theme === 'light' ? 'bg-[#ebebeb] border-black/5' : 'bg-[#2d2d2d] border-white/5'} border-b flex items-center px-4 gap-2 shrink-0`}>
        <button className={`flex items-center gap-2 px-3 py-1.5 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors text-[13px] font-medium`}>
          <IconPlus size={16} /> New
        </button>
        <div className={`w-[1px] h-6 ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'} mx-1`}></div>
        <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors`} title="Cut"><IconCut size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} /></button>
        <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors`} title="Copy"><IconCopy size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} /></button>
        <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors`} title="Paste"><IconClipboard size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} /></button>
        <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors`} title="Rename"><IconEdit size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} /></button>
        <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors`} title="Delete"><IconTrash size={16} className="text-red-500" /></button>
        <div className={`w-[1px] h-6 ${theme === 'light' ? 'bg-black/10' : 'bg-white/10'} mx-1`}></div>
        
        <div className="ml-auto flex items-center gap-1">
          <button 
            className={`p-2 rounded-md transition-colors ${viewMode === 'list' ? (theme === 'light' ? 'bg-black/10' : 'bg-white/10') : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10')}`}
            onClick={() => handleViewModeChange('list')}
          >
            <IconList size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} />
          </button>
          <button 
            className={`p-2 rounded-md transition-colors ${viewMode === 'grid' ? (theme === 'light' ? 'bg-black/10' : 'bg-white/10') : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10')}`}
            onClick={() => handleViewModeChange('grid')}
          >
            <IconLayoutGrid size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} />
          </button>
          <button className={`p-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'} rounded-md transition-colors ml-1`}>
            <IconDots size={16} className={theme === 'light' ? 'text-black/80' : 'text-white/80'} />
          </button>
        </div>
      </div>

      {/* 2. Address Bar */}
      <div className={`h-[48px] ${theme === 'light' ? 'bg-[#f5f5f5] border-black/5' : 'bg-[#1e1e1e] border-white/5'} border-b flex items-center px-4 gap-3 shrink-0`}>
        <div className="flex items-center gap-1">
          <button 
            className={`p-1.5 rounded-md transition-colors ${historyIndex > 0 ? (theme === 'light' ? 'hover:bg-black/5 text-black' : 'hover:bg-white/10 text-white') : (theme === 'light' ? 'text-black/30 cursor-not-allowed' : 'text-white/30 cursor-not-allowed')}`}
            onClick={goBack}
          >
            <IconChevronLeft size={20} stroke={1.5} />
          </button>
          <button 
            className={`p-1.5 rounded-md transition-colors ${historyIndex < history.length - 1 ? (theme === 'light' ? 'hover:bg-black/5 text-black' : 'hover:bg-white/10 text-white') : (theme === 'light' ? 'text-black/30 cursor-not-allowed' : 'text-white/30 cursor-not-allowed')}`}
            onClick={goForward}
          >
            <IconChevronRight size={20} stroke={1.5} />
          </button>
          <button 
            className={`p-1.5 rounded-md transition-colors ${currentPath !== 'Home' ? (theme === 'light' ? 'hover:bg-black/5 text-black' : 'hover:bg-white/10 text-white') : (theme === 'light' ? 'text-black/30 cursor-not-allowed' : 'text-white/30 cursor-not-allowed')}`}
            onClick={goUp}
          >
            <IconArrowUp size={18} stroke={1.5} />
          </button>
          <button className={`p-1.5 rounded-md ${theme === 'light' ? 'hover:bg-black/5 text-black/80' : 'hover:bg-white/10 text-white/80'} transition-colors`} onClick={() => currentPath === 'Home' ? loadDrives() : loadFiles(currentPath)}>
            <IconRefresh size={16} stroke={1.5} />
          </button>
        </div>

        <div className={`flex-1 flex items-center ${theme === 'light' ? 'bg-white border-black/10' : 'bg-[#2d2d2d] border-white/10'} border rounded-md px-3 py-1.5 focus-within:border-blue-500/50 transition-colors`}>
          {currentPath === 'Home' ? (
            <IconHome size={16} className="text-blue-500 mr-2 shrink-0" stroke={1.5} />
          ) : (
            <IconFolderFilled size={16} className="text-yellow-500 mr-2 shrink-0" />
          )}
          <input 
            type="text" 
            value={currentPath} 
            onChange={(e) => setCurrentPath(e.target.value)}
            onKeyDown={(e) => e.key === 'Enter' && navigateTo(currentPath)}
            className={`bg-transparent border-none outline-none w-full text-[13px] ${theme === 'light' ? 'text-black' : 'text-white'}`}
          />
        </div>

        <div className="flex gap-2">
          <select
            value={typeFilter}
            onChange={(e) => handleTypeFilterChange(e.target.value)}
            className={`${theme === 'light' ? 'bg-white border-black/10 text-black' : 'bg-[#2d2d2d] border-white/10 text-white'} border rounded-md px-2 py-1.5 text-[13px] outline-none focus:border-blue-500/50 transition-colors cursor-pointer`}
          >
            <option value="all">All Types</option>
            <option value="image">Images</option>
            <option value="video">Videos</option>
            <option value="audio">Audio</option>
            <option value="document">Documents</option>
            <option value="archive">Archives</option>
          </select>

          <div className={`w-[200px] relative flex items-center ${theme === 'light' ? 'bg-white border-black/10' : 'bg-[#2d2d2d] border-white/10'} border rounded-md px-3 py-1.5 focus-within:border-blue-500/50 transition-colors`}>
            <input 
              type="text" 
              placeholder="Search" 
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className={`bg-transparent border-none outline-none w-full text-[13px] ${theme === 'light' ? 'text-black placeholder-black/40' : 'text-white placeholder-white/40'}`}
            />
            <IconSearch size={16} className={`${theme === 'light' ? 'text-black/40' : 'text-white/40'} ml-2 shrink-0`} stroke={1.5} />
          </div>
        </div>
      </div>

      {/* 3. Main Content Area (Sidebar + File View) */}
      <div className="flex-1 flex overflow-hidden">
        
        {/* Navigation Pane (Sidebar) */}
        <div className={`w-[220px] ${theme === 'light' ? 'bg-[#f5f5f5] border-black/5' : 'bg-[#1e1e1e] border-white/5'} border-r overflow-y-auto custom-scrollbar flex flex-col py-2 shrink-0`}>
          
          {/* Quick Access */}
          <div className="px-2 mb-4">
            <div className={`px-3 py-1 text-[11px] font-semibold ${theme === 'light' ? 'text-black/50' : 'text-white/50'} mb-1`}>Quick Access</div>
            {quickAccess.map((item, i) => (
              <div 
                key={i} 
                onClick={async () => {
                  if (item.path) {
                    navigateTo(item.path);
                  } else {
                    setFolderPickerTarget(item);
                  }
                }}
                onContextMenu={(e) => {
                  e.preventDefault();
                  if (item.id === 'home') return; // Previne menu de contexto para o item Home
                  setContextMenu({ x: e.clientX, y: e.clientY, type: 'quick_access', item });
                }}
                className={`flex items-center gap-3 px-3 py-1.5 rounded-md cursor-pointer transition-colors ${currentPath === item.path && item.path ? (theme === 'light' ? 'bg-black/10' : 'bg-white/10') : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5')}`}
              >
                <item.icon size={16} stroke={1.5} className={item.color} />
                <span className="text-[13px]">{item.label}</span>
              </div>
            ))}
          </div>

          {/* This PC (Drives) */}
          <div className="px-2">
            <div className={`px-3 py-1 text-[11px] font-semibold ${theme === 'light' ? 'text-black/50 hover:text-black/80' : 'text-white/50 hover:text-white/80'} mb-1 flex items-center justify-between group cursor-pointer`} onClick={() => navigateTo('Home')}>
              This PC <IconChevronUp size={14} className="opacity-0 group-hover:opacity-100" />
            </div>
            {drives.map((drive, i) => (
              <div 
                key={i} 
                onClick={() => navigateTo(drive.mount_point)}
                className={`flex items-center gap-3 px-3 py-1.5 rounded-md cursor-pointer transition-colors ${currentPath === drive.mount_point ? (theme === 'light' ? 'bg-black/10' : 'bg-white/10') : (theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5')}`}
              >
                <IconDeviceFloppy size={16} stroke={1.5} className={`${theme === 'light' ? 'text-gray-500' : 'text-gray-300'}`} />
                <span className="text-[13px] truncate">{formatDriveName(drive.name, drive.mount_point)}</span>
              </div>
            ))}
          </div>

        </div>

        {/* File View Area */}
        <div className={`flex-1 ${theme === 'light' ? 'bg-white' : 'bg-[#191919]'} overflow-y-auto custom-scrollbar relative`}>
          
          {loading && (
            <div className={`absolute inset-0 flex items-center justify-center ${theme === 'light' ? 'bg-white/50' : 'bg-[#191919]/50'} z-10`}>
              <div className="animate-spin w-8 h-8 border-2 border-blue-500 border-t-transparent rounded-full"></div>
            </div>
          )}

          {currentPath === 'Home' ? (
            <div className="p-6">
              <h2 className={`text-lg font-semibold mb-4 ${theme === 'light' ? 'text-black' : 'text-white'}`}>Drives</h2>
              <div className="grid grid-cols-1 xl:grid-cols-2 2xl:grid-cols-3 gap-4">
                {drives.map((drive, i) => {
                  const usedSpace = drive.total_space > 0 ? drive.total_space - drive.available_space : 0;
                  const usagePercent = drive.total_space > 0 ? (usedSpace / drive.total_space) * 100 : 0;
                  const isFull = usagePercent > 90;

                  return (
                    <div 
                      key={i} 
                      onClick={() => navigateTo(drive.mount_point)}
                      className={`flex items-center gap-4 ${theme === 'light' ? 'bg-black/5 hover:bg-black/10 border-black/5' : 'bg-white/5 hover:bg-white/10 border-white/5'} border p-4 rounded-xl cursor-pointer transition-all hover:-translate-y-1`}
                    >
                      <IconDeviceFloppy size={44} stroke={1.5} className="text-blue-500 shrink-0" />
                      <div className="flex flex-col w-full">
                        <div className="flex items-center justify-between mb-1">
                          <span className={`font-semibold text-[13px] ${theme === 'light' ? 'text-black' : 'text-white'}`}>{formatDriveName(drive.name, drive.mount_point)}</span>
                        </div>
                        
                        {drive.total_space > 0 ? (
                          <>
                            <div className={`w-full h-[6px] ${theme === 'light' ? 'bg-black/10' : 'bg-black/40'} rounded-full overflow-hidden mb-1 border ${theme === 'light' ? 'border-black/5' : 'border-white/5'}`}>
                              <div 
                                className={`h-full rounded-full ${isFull ? 'bg-red-500' : 'bg-blue-500'}`}
                                style={{ width: `${usagePercent}%` }}
                              ></div>
                            </div>
                            <span className={`text-[11px] ${theme === 'light' ? 'text-black/50' : 'text-white/50'}`}>
                              {formatSize(drive.available_space)} free of {formatSize(drive.total_space)}
                            </span>
                          </>
                        ) : (
                          <span className={`text-[11px] ${theme === 'light' ? 'text-black/50' : 'text-white/50'}`}>Unknown space</span>
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
                      onDoubleClick={() => handleFileDoubleClick(file)}
                      onContextMenu={(e) => {
                        e.preventDefault();
                        setSelectedFile(file.path);
                        setContextMenu({ x: e.clientX, y: e.clientY, type: file.is_dir ? 'folder' : 'file', item: file });
                      }}
                      className={`flex flex-col items-center gap-2 p-3 rounded-lg cursor-pointer transition-colors text-center group ${
                        selectedFile === file.path 
                          ? (theme === 'light' ? 'bg-blue-100 border-blue-300 border text-black' : 'bg-blue-500/30 border-blue-500/50 border text-white')
                          : (theme === 'light' ? 'hover:bg-black/5 text-black border border-transparent' : 'hover:bg-white/10 text-white border border-transparent')
                      }`}
                    >
                      {file.is_dir ? (
                        <IconFolderFilled size={48} className="text-yellow-500 group-hover:scale-105 transition-transform" />
                      ) : isImage && isPicker ? (
                        <IconPhoto size={48} className="text-purple-400 group-hover:scale-105 transition-transform" />
                      ) : (
                        <IconFile size={48} stroke={1.5} className={`${theme === 'light' ? 'text-black/60' : 'text-white/80'} group-hover:scale-105 transition-transform`} />
                      )}
                      <span className="text-[12px] break-words w-full line-clamp-2 leading-tight" title={file.name}>{file.name}</span>
                    </div>
                  )})}
                </div>
              ) : (
                <table className="w-full text-left border-collapse table-fixed">
                  <thead className={`sticky top-0 ${theme === 'light' ? 'bg-white shadow-[0_1px_0_rgba(0,0,0,0.05)] text-black/70' : 'bg-[#191919] shadow-[0_1px_0_rgba(255,255,255,0.05)] text-white/70'} z-10`}>
                    <tr className="text-[12px] select-none">
                      <th className={`font-normal px-4 py-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5'} cursor-pointer transition-colors`} onClick={() => handleSort('name')}>
                        <div className="flex items-center gap-2">
                          Name {sortBy === 'name' && (sortOrder === 'asc' ? <IconChevronUp size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} /> : <IconChevronDown size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} />)}
                        </div>
                      </th>
                      <th className={`font-normal px-4 py-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5'} cursor-pointer w-40 transition-colors`} onClick={() => handleSort('date')}>
                        <div className="flex items-center gap-2">
                          Date modified {sortBy === 'date' && (sortOrder === 'asc' ? <IconChevronUp size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} /> : <IconChevronDown size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} />)}
                        </div>
                      </th>
                      <th className={`font-normal px-4 py-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5'} cursor-pointer w-32 transition-colors`} onClick={() => handleSort('type')}>
                        <div className="flex items-center gap-2">
                          Type {sortBy === 'type' && (sortOrder === 'asc' ? <IconChevronUp size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} /> : <IconChevronDown size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} />)}
                        </div>
                      </th>
                      <th className={`font-normal px-4 py-2 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/5'} cursor-pointer w-32 transition-colors`} onClick={() => handleSort('size')}>
                        <div className="flex items-center gap-2">
                          Size {sortBy === 'size' && (sortOrder === 'asc' ? <IconChevronUp size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} /> : <IconChevronDown size={14} className={`${theme === 'light' ? 'text-black' : 'text-white'} shrink-0`} />)}
                        </div>
                      </th>
                    </tr>
                  </thead>
                  <tbody>
                    {filteredAndSortedFiles.map((file, i) => (
                      <tr 
                        key={i} 
                        onClick={() => handleFileClick(file)}
                        onDoubleClick={() => handleFileDoubleClick(file)}
                        onContextMenu={(e) => {
                          e.preventDefault();
                          setSelectedFile(file.path);
                          setContextMenu({ x: e.clientX, y: e.clientY, type: file.is_dir ? 'folder' : 'file', item: file });
                        }}
                        className={`cursor-pointer border-b text-[13px] transition-colors ${
                          selectedFile === file.path 
                            ? (theme === 'light' ? 'bg-blue-100 border-blue-300 text-black' : 'bg-blue-500/30 border-blue-500/50 text-white')
                            : (theme === 'light' ? 'hover:bg-black/5 text-black border-transparent' : 'hover:bg-white/5 text-white border-transparent')
                        }`}
                      >
                        <td className="px-4 py-2">
                          <div className="flex items-center gap-3 w-full overflow-hidden">
                            <div className="shrink-0 flex items-center justify-center">
                              {file.is_dir ? <IconFolderFilled size={16} className="text-yellow-500" /> : <IconFile size={16} stroke={1.5} className={`${theme === 'light' ? 'text-black/60' : 'text-white/60'}`} />}
                            </div>
                            <span className="truncate" title={file.name}>{file.name}</span>
                          </div>
                        </td>
                        <td className={`px-4 py-2 ${theme === 'light' ? 'text-black/50' : 'text-white/50'} truncate`}>{file.modified_at ? new Date(file.modified_at * 1000).toLocaleDateString() : ''}</td>
                        <td className={`px-4 py-2 ${theme === 'light' ? 'text-black/50' : 'text-white/50'} truncate`}>{file.is_dir ? 'File folder' : getFileExtension(file.name).toUpperCase() + ' File'}</td>
                        <td className={`px-4 py-2 ${theme === 'light' ? 'text-black/50' : 'text-white/50'} truncate`}>{file.is_dir ? '' : formatSize(file.size)}</td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              )}
              {filteredAndSortedFiles.length === 0 && !loading && (
                <div className={`flex flex-col items-center justify-center py-20 ${theme === 'light' ? 'text-black/30' : 'text-white/30'}`}>
                  <IconFolderFilled size={64} className="mb-4 opacity-50" />
                  <p>This folder is empty.</p>
                </div>
              )}
            </div>
          )}
        </div>
      </div>
      
      {/* Status Bar */}
      {!isPicker && (
        <div className={`h-[24px] ${theme === 'light' ? 'bg-[#0078D7]' : 'bg-[#0078D7]'} text-white flex items-center px-4 shrink-0 text-[11px]`}>
          {currentPath === 'Home' 
            ? `${drives.length} item(s)` 
            : `${filteredAndSortedFiles.length} item(s)`
          }
        </div>
      )}

      {/* Bottom Footer for Folder Picker */}
      {isPicker && pickerMode === 'folder' && (
        <div className={`h-[60px] border-t flex items-center justify-between px-4 shrink-0 ${theme === 'light' ? 'bg-[#f5f5f5] border-black/10' : 'bg-[#2d2d2d] border-white/10'}`}>
          <div className="text-[13px] truncate flex-1 mr-4">
            Folder: <span className="font-semibold">{currentPath}</span>
          </div>
          <div className="flex gap-2">
            <button 
              className={`px-4 py-1.5 rounded-md text-[13px] font-medium transition-colors ${theme === 'light' ? 'bg-black/5 hover:bg-black/10 text-black' : 'bg-white/10 hover:bg-white/20 text-white'}`}
              onClick={() => onClosePicker && onClosePicker()}
            >
              Cancel
            </button>
            <button 
              className="px-4 py-1.5 rounded-md text-[13px] font-medium bg-blue-500 hover:bg-blue-600 text-white transition-colors disabled:opacity-50"
              disabled={currentPath === 'Home'}
              onClick={() => {
                if (currentPath !== 'Home' && onFolderSelect) {
                  onFolderSelect(currentPath);
                }
              }}
            >
              Select Folder
            </button>
          </div>
        </div>
      )}

      {/* Folder Picker Modal */}
      {folderPickerTarget && (
        <div className="absolute inset-0 z-[9999] flex items-center justify-center bg-black/50 p-4">
          <div className={`w-full max-w-4xl h-[90%] flex flex-col rounded-xl overflow-hidden shadow-2xl ${theme === 'light' ? 'bg-white' : 'bg-[#1e1e1e]'}`}>
            <div className={`px-4 py-3 border-b flex justify-between items-center ${theme === 'light' ? 'border-black/10' : 'border-white/10'}`}>
              <span className="font-semibold text-[14px]">Select folder for {folderPickerTarget.label}</span>
              <button onClick={() => setFolderPickerTarget(null)} className={`p-1 rounded-md ${theme === 'light' ? 'hover:bg-black/10' : 'hover:bg-white/10'}`}><IconX size={18} /></button>
            </div>
            <div className="flex-1 relative overflow-hidden">
               <FileExplorerBase 
                 isPicker={true} 
                 pickerMode="folder" 
                 onFolderSelect={async (path) => {
                    const newQuickAccess = quickAccess.map(q => q.id === folderPickerTarget.id ? { ...q, path } : q);
                    setQuickAccess(newQuickAccess);
                    await store.set('explorer_quick_access', newQuickAccess);
                    await store.save();
                    setFolderPickerTarget(null);
                 }}
                 onClosePicker={() => setFolderPickerTarget(null)}
               />
            </div>
          </div>
        </div>
      )}

      {/* Context Menu */}
      {contextMenu && createPortal(
        <div 
          className={`fixed z-[999999] w-48 py-1 rounded-md shadow-xl border text-[13px] ${
            theme === 'light' ? 'bg-white border-black/10 text-black' : 'bg-[#2d2d2d] border-white/10 text-white'
          }`}
          style={{ top: contextMenu.y, left: contextMenu.x }}
          onClick={(e) => e.stopPropagation()}
          onContextMenu={(e) => {
            e.preventDefault();
            e.stopPropagation();
          }}
        >
          {contextMenu.type === 'quick_access' && (
            <>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handleChangeQuickAccessPath(contextMenu.item); setContextMenu(null); }}
              >
                Change Location
              </div>
              {!contextMenu.item.isSystem && (
                <div 
                  className={`px-4 py-1.5 cursor-pointer text-red-500 ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                  onClick={() => { handleUnpinQuickAccess(contextMenu.item); setContextMenu(null); }}
                >
                  Unpin from Quick Access
                </div>
              )}
            </>
          )}
          {contextMenu.type === 'folder' && (
            <>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handleFileDoubleClick(contextMenu.item); setContextMenu(null); }}
              >
                Open
              </div>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handlePinToQuickAccess(contextMenu.item); setContextMenu(null); }}
              >
                Pin to Quick Access
              </div>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handleCreateDesktopShortcut(contextMenu.item); setContextMenu(null); }}
              >
                Create Desktop Shortcut
              </div>
            </>
          )}
          {contextMenu.type === 'file' && (
            <>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handleFileDoubleClick(contextMenu.item); setContextMenu(null); }}
              >
                Open
              </div>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { handleCreateDesktopShortcut(contextMenu.item); setContextMenu(null); }}
              >
                Create Desktop Shortcut
              </div>
              <div 
                className={`px-4 py-1.5 cursor-pointer ${theme === 'light' ? 'hover:bg-black/5' : 'hover:bg-white/10'}`}
                onClick={() => { navigator.clipboard.writeText(contextMenu.item.path); setContextMenu(null); }}
              >
                Copy Path
              </div>
            </>
          )}
        </div>,
        document.body
      )}

    </div>
  );
};

export default function FileExplorer({ onOpenInApp }: { onOpenInApp?: (baseId: string, props: any) => void }) {
  return <FileExplorerBase onOpenInApp={onOpenInApp} />;
}