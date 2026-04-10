import React, { createContext, useContext, useState, useEffect } from 'react';
import { LazyStore } from '@tauri-apps/plugin-store';
import { invoke } from '@tauri-apps/api/core';

type Theme = 'dark' | 'light';

interface ThemeContextType {
  theme: Theme;
  setTheme: (theme: Theme) => void;
  wallpaper: string;
  setWallpaper: (wallpaper: string, isLocalPath?: boolean) => void;
  isLoading: boolean;
}

export const ThemeContext = createContext<ThemeContextType>({
  theme: 'dark',
  setTheme: () => {},
  wallpaper: '/wallpaper1.png',
  setWallpaper: () => {},
  isLoading: true,
});

export const useTheme = () => useContext(ThemeContext);

const store = new LazyStore('settings.json');

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => {
  const [theme, setThemeState] = useState<Theme>('dark');
  const [wallpaper, setWallpaperState] = useState<string>('/wallpaper1.png');
  const [isLoading, setIsLoading] = useState(true);

  // Load settings from store on startup
  useEffect(() => {
    const loadSettings = async () => {
      try {
        const savedTheme = await store.get<Theme>('theme');
        if (savedTheme) setThemeState(savedTheme);

        const savedWallpaperPath = await store.get<string>('wallpaper_path');
        if (savedWallpaperPath) {
          // If it's a local file path, load the bytes and create an object URL
          if (!savedWallpaperPath.startsWith('/')) {
             try {
                const bytes: number[] = await invoke('read_file_bytes', { path: savedWallpaperPath });
                const blob = new Blob([new Uint8Array(bytes)]);
                const url = URL.createObjectURL(blob);
                setWallpaperState(url);
             } catch (e) {
                console.error('Failed to load saved wallpaper path:', e);
             }
          } else {
            // It's a bundled default wallpaper
            setWallpaperState(savedWallpaperPath);
          }
        }
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
    await store.set('theme', newTheme);
    await store.save();
  };

  const setWallpaper = async (newWallpaper: string, isLocalPath: boolean = false) => {
    setWallpaperState(newWallpaper);
    // Only save to store if we have the absolute path, not a blob URL
    // If it's a default wallpaper (e.g. /wallpaper1.png), we can just save it
    if (isLocalPath || newWallpaper.startsWith('/')) {
        await store.set('wallpaper_path', newWallpaper);
        await store.save();
    }
  };

  return (
    <ThemeContext.Provider value={{ theme, setTheme, wallpaper, setWallpaper, isLoading }}>
      {!isLoading && children}
    </ThemeContext.Provider>
  );
};