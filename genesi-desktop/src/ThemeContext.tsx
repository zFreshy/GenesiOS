import React, { createContext, useContext, useState, useEffect } from 'react';
import { LazyStore } from '@tauri-apps/plugin-store';
import { invoke, isTauri } from '@tauri-apps/api/core';

type Theme = 'dark' | 'light';

interface ThemeContextType {
  theme: Theme;
  setTheme: (theme: Theme) => void;
  wallpapers: Record<string, string>; // { "display-0": "url", "display-1": "url" }
  wallpaperHistory: string[]; // absolute paths
  setWallpaper: (wallpaper: string, monitorId: string | 'all', isLocalPath?: boolean) => void;
  isLoading: boolean;
}

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'dark',
  setTheme: () => {},
  wallpapers: {},
  wallpaperHistory: ['/wallpaper1.png'],
  setWallpaper: () => {},
  isLoading: true,
});

export const useTheme = () => useContext(ThemeContext);

const store = new LazyStore('settings.json');

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [theme, setThemeState] = useState<Theme>('dark');
  const [wallpapers, setWallpapersState] = useState<Record<string, string>>({});
  const [wallpaperHistory, setWallpaperHistory] = useState<string[]>(['/wallpaper1.png']);
  const [isLoading, setIsLoading] = useState(true);

  // Helper to load image bytes to blob URL
  const loadBlobUrl = async (path: string) => {
    if (path.startsWith('/')) return path; // Default bundled wallpaper
    if (!isTauri()) return path; // No-op in browser

    try {
      const bytes: number[] = await invoke('read_file_bytes', { path });
      const blob = new Blob([new Uint8Array(bytes)]);
      return URL.createObjectURL(blob);
    } catch (e) {
      console.error('Failed to load saved wallpaper path:', path, e);
      return '/wallpaper1.png'; // fallback
    }
  };

  // Load settings from store on startup
  useEffect(() => {
    const loadSettings = async () => {
      try {
        if (!isTauri()) {
          setWallpapersState({ 'all': '/wallpaper1.png' });
          setIsLoading(false);
          return;
        }

        const savedTheme = await store.get<Theme>('theme');
        if (savedTheme) setThemeState(savedTheme);

        const savedHistory = await store.get<string[]>('wallpaper_history');
        if (savedHistory && savedHistory.length > 0) setWallpaperHistory(savedHistory);

        const savedWallpapersPaths = await store.get<Record<string, string>>('wallpapers_paths') || {};
        
        // Se não houver wallpapers salvos, define o fallback padrão para 'all'
        if (Object.keys(savedWallpapersPaths).length === 0) {
          savedWallpapersPaths['all'] = '/wallpaper1.png';
        }

        // Convert absolute paths to Blob URLs for React to render
        const loadedWallpapers: Record<string, string> = {};
        for (const [monitorId, path] of Object.entries(savedWallpapersPaths)) {
          loadedWallpapers[monitorId] = await loadBlobUrl(path);
        }
        setWallpapersState(loadedWallpapers);

      } catch (error) {
        console.error('Failed to load settings:', error);
      } finally {
        setIsLoading(false);
      }
    };

    loadSettings();
  }, []);

  const setTheme = async (newTheme: Theme) => {
    setThemeState(newTheme);
    if (!isTauri()) return;
    await store.set('theme', newTheme);
    await store.save();
  };

  const setWallpaper = async (newWallpaper: string, monitorId: string | 'all', isLocalPath: boolean = false) => {
    // Determine the actual path/url to save and render
    const actualPath = newWallpaper;
    const blobUrl = isLocalPath ? await loadBlobUrl(newWallpaper) : newWallpaper;

    // 1. Update Visual State
    setWallpapersState(prev => ({
      ...prev,
      [monitorId]: blobUrl
    }));

    if (!isTauri()) return;

    // 2. Persist to Disk (using the actual path, not the blob url)
    if (isLocalPath || actualPath.startsWith('/')) {
      // Update history (keep last 5)
      setWallpaperHistory(prev => {
        const newHistory = [actualPath, ...prev.filter(p => p !== actualPath)].slice(0, 5);
        store.set('wallpaper_history', newHistory).then(() => store.save());
        return newHistory;
      });

      // Update monitor mapping
      const savedWallpapersPaths = await store.get<Record<string, string>>('wallpapers_paths') || {};
      savedWallpapersPaths[monitorId] = actualPath;
      await store.set('wallpapers_paths', savedWallpapersPaths);
      await store.save();
    }
  };

  return (
    <ThemeContext.Provider value={{ theme, setTheme, wallpapers, wallpaperHistory, setWallpaper, isLoading }}>
      {!isLoading && children}
    </ThemeContext.Provider>
  );
};